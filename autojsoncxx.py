#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# The MIT License (MIT)
#
# Copyright (c) 2014 Siyuan Ren (netheril96@gmail.com)
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in all
# copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.


import re
import argparse
import sys
import json
import os


def hard_escape(text):
    if isinstance(text, str):
        text = text.encode('utf-8')

    def escape(char):
        return '\\x{:02x}'.format(char)

    return '"' + ''.join(escape(char) for char in text) + '"'


class ClassInfo:
    def __init__(self, record):
        self._name = record['name']
        self._members = [MemberInfo(r) for r in record['members']]
        self._strict = record.get('strict', False)
        self._namespace = record.get("namespace", None)

    def member_declarations(self):
        return '\n'.join(m.type_name() + ' ' + m.variable_name() + ';' for m in self.members())

    def initializer_list(self):
        return ','.join('{}({})'.format(m.variable_name(), m.constructor_args()) for m in self.members())

    def class_definition(self):
        class_def = 'class {name} {{\npublic:\n {declarations}\n explicit {name}():{init}{{}}\n}};' \
            .format(name=self.name(), declarations=self.member_declarations(), init=self.initializer_list())
        if self._namespace is not None:
            return 'namespace {} {{ {} }}\n'.format(self._namespace, class_def)
        return class_def

    def name(self):
        return self._name

    def qualified_name(self):
        if self._namespace is None:
            return '::' + self.name()
        return self._namespace + '::' + self.name()

    def members(self):
        return self._members

    def is_strict(self):
        return self._strict


class InvalidIdentifier(Exception):
    def __init__(self, identifier):
        self._identifier = identifier

    def __str__(self):
        return "Invalid string for C++ identifier: " + repr(self._identifier)


class UnsupportedTypeError(Exception):
    def __init__(self, type_name):
        self._type_name = type_name

    def __str__(self):
        return "Unsupported C++ type: " + repr(self._type_name)


class MemberInfo:
    def __init__(self, record):
        self._record = record

        if '*' in self.type_name() or '&' in self.type_name():
            raise UnsupportedTypeError(self.type_name())

        if not re.match(r'[A-Za-z]\w*|_[a-z0-9]\w*', self.variable_name()):
            raise InvalidIdentifier(self.variable_name())

    def type_name(self):
        return self._record[0]

    def variable_name(self):
        return self._record[1]

    def json_key(self):
        try:
            return self._record[2]['json_key']
        except (IndexError, KeyError):
            return self.variable_name()

    def is_required(self):
        try:
            return self._record[2]['required']
        except (IndexError, KeyError):
            return False

    def default(self):
        try:
            return self._record[2]['default']
        except (IndexError, KeyError):
            return None

    def constructor_args(self):
        def to_cpp_repr(args) -> str:
            if args is None:
                return ''
            elif args is True:
                return 'true'
            elif args is False:
                return 'false'
            elif isinstance(args, str):
                return hard_escape(args)
            elif isinstance(args, int) or isinstance(args, float):
                return str(args)
            else:
                return ', '.join(to_cpp_repr(a) for a in args)

        return to_cpp_repr(self.default())


class MainCodeGenerator:
    def __init__(self, members_info):
        self.members_info = members_info

    def handler_declarations(self):
        return '\n'.join('SAXEventHandler< {} > handler_{};'.format(m.type_name(), i)
                         for i, m in enumerate(self.members_info))

    def handler_initializers(self):
        return '\n'.join(', handler_{}(&obj->{})'.format(i, m.variable_name())
                         for i, m in enumerate(self.members_info))

    def key_event_handling(self):
        return '\n'.join('else if (utility::string_equal(str, length, {}))\n    state={};'
                             .format(hard_escape(m.json_key()), i) for i, m in enumerate(self.members_info))

    def event_forwarding(self, call_text):
        return '\n\n'.join('case {i}:\n    return checked_event_forwarding(handler_{i}.{call});'
                               .format(i=i, call=call_text) for i in range(len(self.members_info)))

    def error_reaping(self):
        return '\n'.join('case {0}:\n     handler_{0}.ReapError(errs); break;'.format(i)
                         for i in range(len(self.members_info)))

    def data_serialization(self):
        return '\n'.join('w.Key("{}"); Serializer< Writer_6FD4E37439E0A95BB8A3, {} >()(w, value.{});'
                             .format(m.json_key(), m.type_name(), m.variable_name())
                         for m in self.members_info)

    def current_member_name(self):
        return '\n'.join('case {}:\n    return "{}";'.format(i, m.variable_name())
                         for i, m in enumerate(self.members_info))


def build_class(template, class_info):
    gen = MainCodeGenerator(class_info.members())

    result = template
    result = re.sub(r'/\*\s*class definition\s*\*/', class_info.class_definition(), result)
    result = re.sub(r'/\*\s*list of handlers\s*\*/', gen.handler_declarations(), result)
    result = re.sub(r'/\*\s*init handlers\s*\*/', gen.handler_initializers(), result)
    result = re.sub(r'/\*\s*serialize all members\s*\*/', gen.data_serialization(), result)
    result = re.sub(r'/\*\s*change state\s*\*/', gen.key_event_handling(), result)
    result = re.sub(r'/\*\s*reap error\s*\*/', gen.error_reaping(), result)
    result = re.sub(r'/\*\s*get member name\s*\*/', gen.current_member_name(), result)

    def evaluate(match):
        return gen.event_forwarding(match.group(1))

    result = re.sub(r'/\*\s*forward (.*?) to members\s*\*/', evaluate, result)
    result = result.replace('TypeName_A27885315D2EA6F8BEB7', class_info.qualified_name())

    return result


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("definition", help='definition file for classes')
    parser.add_argument('-o', '--out', help='output file name (defaults to stdout)', default=None)
    parser.add_argument('--template', help='location of the template file', default=None)
    args = parser.parse_args()

    if args.template is None:
        args.template = os.path.join(os.path.dirname(os.path.abspath(__file__)), 'code_template')

    with open(args.template) as f:
        template = f.read()

    def process_file(output):
        with open(args.definition) as f:
            raw_record = json.load(f)

        output.write('#pragma once\n\n')

        if isinstance(raw_record, list):
            for r in raw_record:
                output.write(build_class(template, ClassInfo(r)))
        else:
            output.write(build_class(template, ClassInfo(raw_record)))

    if args.out is None:
        process_file(sys.stdout)
    else:
        with open(args.out, 'w') as f:
            process_file(f)


if __name__ == '__main__':
    main()