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


class InvalidDefinitionError(Exception):
    pass


class InvalidIdentifier(InvalidDefinitionError):
    def __init__(self, identifier):
        self.identifier = identifier

    def __str__(self):
        return "Invalid string for C++ identifier: " + repr(self.identifier)


class InvalidNamespace(InvalidDefinitionError):
    def __init__(self, namespace):
        self.namespace = namespace

    def __str__(self):
        return "Invalid namespace: " + repr(self.namespace)


class UnrecognizedOption(InvalidDefinitionError):
    def __init__(self, option):
        self.option = option

    def __str__(self):
        return "Unrecognized option: " + repr(self.option)


class UnsupportedTypeError(InvalidDefinitionError):
    def __init__(self, type_name):
        self.type_name = type_name

    def __str__(self):
        return "Unsupported C++ type: " + repr(self.type_name)


def hard_escape(text):
    def escape(char):
        return '\\x{:02x}'.format(char)

    return '"' + ''.join(escape(char) for char in text) + '"'


def check_identifier(identifier):
    if not re.match(r'^[A-Za-z_]\w*$', identifier):
        raise InvalidIdentifier(identifier)


class ClassInfo:
    accept_options = {"name", "namespace", "parse_mode", "members"}

    def __init__(self, record):
        self._name = record['name']
        self._members = [MemberInfo(r) for r in record['members']]
        self._strict = record.get('parse_mode', '') == 'strict'
        self._namespace = record.get("namespace", None)

        check_identifier(self._name)

        if self._namespace is not None and not re.match(r'^[A-Za-z_]\w*(?:::[A-Za-z_]\w*)*$', self._namespace):
            raise InvalidNamespace(self._namespace)

        for op in record:
            if op not in ClassInfo.accept_options:
                raise UnrecognizedOption(op)

    def member_declarations(self):
        return '\n'.join(m.type_name() + ' ' + m.variable_name() + ';' for m in self.members())

    def initializer_list(self, is_default=True):
        if is_default:
            return ', '.join('{}({})'.format(m.variable_name(), m.constructor_args()) for m in self.members())
        else:
            return ', '.join('{0}({0})'.format(m.variable_name()) for m in self.members())

    def constructor(self, is_default=True):
        if is_default:
            args = ''
        else:
            args = ', '.join('const {}& {}'.format(m.type_name(), m.variable_name()) for m in self.members())
        return 'explicit {name}({args}):{init} {{}}\n'.format(name=self.name(), args=args,
                                                              init=self.initializer_list(is_default))

    def class_definition(self):
        class_def = 'struct {name} {{\n {declarations}\n\n{constructor1}\n\n{constructor2} \n}};' \
            .format(name=self.name(), declarations=self.member_declarations(),
                    constructor1=self.constructor(True), constructor2=self.constructor(False))

        if self._namespace is not None:
            for space in reversed(self._namespace.split('::')):
                if space:
                    class_def = 'namespace {} {{ {} }}\n'.format(space, class_def)
        return class_def

    def name(self):
        return self._name

    def qualified_name(self):
        if self._namespace is None:
            return '::' + self.name()
        return self._namespace + '::' + self.name()

    def members(self):
        return self._members

    def strict_parsing(self):
        return self._strict

    def namespace(self):
        return self._namespace


def to_cpp_repr(args):
    if args is None:
        return ''
    elif args is True:
        return 'true'
    elif args is False:
        return 'false'
    elif isinstance(args, str):
        return hard_escape(args.encode('utf-8'))
    elif isinstance(args, int) or isinstance(args, float):
        return str(args)
    else:
        return ', '.join(to_cpp_repr(a) for a in args)


class MemberInfo:
    accept_options = {'default', 'required', 'json_key'}

    def __init__(self, record):
        self._record = record

        if '*' in self.type_name() or '&' in self.type_name():
            raise UnsupportedTypeError(self.type_name())

        check_identifier(self.variable_name())

        if len(record) > 3:
            raise UnrecognizedOption(record[3:])

        if len(record) == 3:
            for op in record[2]:
                if op not in MemberInfo.accept_options:
                    raise UnrecognizedOption(op)

    def type_name(self):
        return self._record[0]

    def variable_name(self):
        return self._record[1]

    def json_key(self):
        try:
            return self._record[2]['json_key'].encode('utf-8')
        except (IndexError, KeyError):
            return self.variable_name().encode('utf-8')

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
        return to_cpp_repr(self.default())

    def set_flag_statement(self, flag):
        if self.is_required():
            return 'has_{} = {};'.format(self.variable_name(), flag)
        else:
            return ''


class MainCodeGenerator:
    def __init__(self, class_info):
        self.members_info = class_info.members()
        self.class_info = class_info

    def handler_declarations(self):
        return '\n'.join('SAXEventHandler< {} > handler_{};'.format(m.type_name(), i)
                         for i, m in enumerate(self.members_info))

    def handler_initializers(self):
        return '\n'.join(', handler_{}(&obj->{})'.format(i, m.variable_name())
                         for i, m in enumerate(self.members_info))

    def flags_declaration(self):
        return '\n'.join('bool has_{};'.format(m.variable_name()) for m in self.members_info if m.is_required())

    def flags_reset(self):
        return '\n'.join(m.set_flag_statement("false") for m in self.members_info)

    def post_validation(self):
        return '\n'.join('if (!has_{0}) set_missing_required("{0}");'
                             .format(m.variable_name()) for m in self.members_info if m.is_required())

    def key_event_handling(self):
        return '\n'.join('else if (utility::string_equal(str, length, {key}, {key_length}))\n\
                    {{ state={state}; {check} }}'
                             .format(key=hard_escape(m.json_key()), key_length=len(m.json_key()),
                                     state=i, check=m.set_flag_statement("true"))
                         for i, m in enumerate(self.members_info))

    def event_forwarding(self, call_text):
        return '\n\n'.join('case {i}:\n    return checked_event_forwarding(handler_{i}.{call});'
                               .format(i=i, call=call_text) for i in range(len(self.members_info)))

    def error_reaping(self):
        return '\n'.join('case {0}:\n     handler_{0}.ReapError(errs); break;'.format(i)
                         for i in range(len(self.members_info)))

    def data_serialization(self):
        return '\n'.join('w.Key({}); Serializer< Writer_6FD4E37439E0A95BB8A3, {} >()(w, value.{});'
                             .format(hard_escape(m.json_key()), m.type_name(), m.variable_name())
                         for m in self.members_info)

    def current_member_name(self):
        return '\n'.join('case {}:\n    return "{}";'.format(i, m.variable_name())
                         for i, m in enumerate(self.members_info))

    def unknown_key_handling(self):
        if self.class_info.strict_parsing():
            return 'the_error.reset(new error::UnknownFieldError(str, length)); return false;'
        else:
            return 'return true;'


def build_class(template, class_info):
    gen = MainCodeGenerator(class_info)

    result = template
    result = re.sub(r'/\*\s*class definition\s*\*/', class_info.class_definition(), result)
    result = re.sub(r'/\*\s*list of declarations\s*\*/', gen.handler_declarations() + gen.flags_declaration(), result)
    result = re.sub(r'/\*\s*init\s*\*/', gen.handler_initializers(), result)
    result = re.sub(r'/\*\s*serialize all members\s*\*/', gen.data_serialization(), result)
    result = re.sub(r'/\*\s*change state\s*\*/', gen.key_event_handling(), result)
    result = re.sub(r'/\*\s*reap error\s*\*/', gen.error_reaping(), result)
    result = re.sub(r'/\*\s*get member name\s*\*/', gen.current_member_name(), result)
    result = re.sub(r'/\*\s*validation\s*\*/', gen.post_validation(), result)
    result = re.sub(r'/\*\s*reset flags\s*\*/', gen.flags_reset(), result)
    result = re.sub(r'/\*\s*handle unknown keys?\s*\*/', gen.unknown_key_handling(), result)

    def evaluate(match):
        return gen.event_forwarding(match.group(1))

    result = re.sub(r'/\*\s*forward (.*?) to members\s*\*/', evaluate, result)
    result = result.replace('TypeName_A27885315D2EA6F8BEB7', class_info.qualified_name())

    return result


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("definition", help='definition file for classes')
    parser.add_argument('-o', '--out', help='output file name', default=None)
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
        args.out = os.path.basename(args.definition)
        args.out = os.path.splitext(args.out)[0] + '.hpp'

    with open(args.out, 'w') as f:
        process_file(f)


if __name__ == '__main__':
    main()