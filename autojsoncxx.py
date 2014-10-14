#!/usr/bin/env python
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

from __future__ import unicode_literals
from __future__ import print_function

import re
import argparse
import os
import hashlib
import sys

is_python2 = sys.version_info.major == 2

if is_python2:
    import io
    open = io.open
    str = unicode

try:
    import simplejson as json
except ImportError:
    import json

try:
    import parsimonious

    # PEG grammar for parsing the C++ type name we support
    # Note that raw pointer, reference, array, void, enum, function and pointer-to-member types are not supported
    grammar = parsimonious.Grammar(r'''
        type = (space cv_type space "<" space type_list space ">" space) / ( space cv_type space )
        type_list = (type space "," space type_list) / type / space

        cv_type = c_and_v_type / c_or_v_type / simple_type
        c_and_v_type = ("const" space "volatile" space simple_type) / ("volatile" space "const" space simple_type)
        c_or_v_type = ("const" space simple_type) / ("volatile" space simple_type)

        simple_type = spaced_type / ("::"? identifier ("::" identifier)*)
        spaced_type = sign_type / long_type
        sign_type = ("unsigned" / "signed")  space ( ("long" space "long"? space "int"?) / "int" / "char")
        long_type = ("long" space "long" space "int") / ("long" space "long") / ("long" space "int")

        identifier = ~"[A-Za-z_][A-Za-z_0-9]*"
        space = ~"[ \t]*"
        ''')

    def extract_simple_type(node):
        if node.expr_name == 'simple_type':
            yield node.text.lstrip(':')

        for sub_node in node.children:
            for value in extract_simple_type(sub_node):
                yield value

    def check_for_unknown_basic_types(name, cache):
        """
        :param name: the full name of the type to check
        :param cache: the names that has been encountered so far; updated after function returns
        :return: a list of unknown types
        """
        node = grammar.parse(name)
        simple_types = set(extract_simple_type(node))
        unknowns = simple_types - cache
        cache.update(simple_types)
        return unknowns


except ImportError:
    parsimonious = None


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


if is_python2:
    def hard_escape(text):
        def escape(char):
            return '\\x{:02x}'.format(ord(char))

        return '"' + ''.join(escape(char) for char in text) + '"'
else:
    def hard_escape(text):
        def escape(char):
            return '\\x{:02x}'.format(char)

        return '"' + ''.join(escape(char) for char in text) + '"'


def check_identifier(identifier):
    if not re.match(r'^[A-Za-z_]\w*$', identifier):
        raise InvalidIdentifier(identifier)


class ClassInfo:
    accept_options = {"name", "namespace", "parse_mode", "members", "constructor_code", "comment"}

    def __init__(self, record):
        self._name = record['name']
        self._members = [MemberInfo(r) for r in record['members']]
        self._strict = record.get('parse_mode', '') == 'strict'
        self._namespace = record.get("namespace", None)
        self._constructor_code = record.get("constructor_code", "")

        check_identifier(self._name)

        if self._namespace is not None and not re.match(r'^(?:::)?[A-Za-z_]\w*(?:::[A-Za-z_]\w*)*$', self._namespace):
            raise InvalidNamespace(self._namespace)

        for op in record:
            if op not in ClassInfo.accept_options:
                raise UnrecognizedOption(op)

    def member_declarations(self):
        return '\n'.join(m.type_name() + ' ' + m.variable_name() + ';' for m in self.members())

    def initializer_list(self):
        return ', '.join('{0}({1})'.format(m.variable_name(), m.constructor_args()) for m in self.members())

    def constructor(self):
        return 'explicit {name}():{init} {{ {code} }}\n'.format(name=self.name(), init=self.initializer_list(),
                                                                code=self._constructor_code)

    def class_definition(self):
        class_def = 'struct {name} {{\n {declarations}\n\n{constructor}\n\n \n}};' \
            .format(name=self.name(), declarations=self.member_declarations(),
                    constructor=self.constructor())

        if self._namespace is not None:
            for space in reversed(self._namespace.split('::')):
                if space:
                    class_def = 'namespace {} {{ {} }}\n'.format(space, class_def)
        return class_def

    def name(self):
        return self._name

    def qualified_name(self):
        if self.namespace() is None:
            return '::' + self.name()
        if self.namespace().startswith('::'):
            return self.namespace() + '::' + self.name()
        else:
            return '::' + self.namespace() + '::' + self.name()

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
        raise UnrecognizedOption("default=" + repr(args))


class MemberInfo:
    accept_options = {'default', 'required', 'json_key', 'comment'}

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

    def writer_type_name(self):
        return "Writer" + hashlib.sha256(self.class_info.qualified_name().encode()).hexdigest()

    def data_serialization(self):
        return '\n'.join('w.Key({}); Serializer< {}, {} >()(w, value.{});'
                             .format(hard_escape(m.json_key()), self.writer_type_name(),
                                     m.type_name(), m.variable_name())
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

    replacement = {
        "class definition": class_info.class_definition(),
        "list of declarations": gen.handler_declarations() + gen.flags_declaration(),
        "init": gen.handler_initializers(),
        "serialize all members": gen.data_serialization(),
        "change state": gen.key_event_handling(),
        "reap error": gen.error_reaping(),
        "get member name": gen.current_member_name(),
        "validation": gen.post_validation(),
        "reset flags": gen.flags_reset(),
        "handle unknown key": gen.unknown_key_handling(),
        "TypeName": class_info.qualified_name(),
        "Writer": gen.writer_type_name()}

    def evaluate(match):
        try:
            return replacement[match.group(1)]
        except KeyError:
            match = re.match(r'forward (.*?) to members', match.group(1))
            if match:
                return gen.event_forwarding(match.group(1))
            else:
                raise

    return re.sub(r'/\*\s*(.*?)\s*\*/', evaluate, template)


def check_all_members(class_info, cache):
    for m in class_info.members():
        try:
            unknowns = check_for_unknown_basic_types(m.type_name(), cache)
            for u in unknowns:
                print("Warning:", "The type", repr(u), "may not be recognized", file=sys.stderr)
                print("\tReferenced from variable", repr(m.variable_name()),
                      "in class", repr(class_info.qualified_name()), "\n", file=sys.stderr)
        except parsimonious.ParseError:
            print("Warning:", "The type", repr(m.type_name()), "is not valid", file=sys.stderr)
            print("\tReferenced from variable", repr(m.variable_name()),
                  "in class", repr(class_info.qualified_name()), "\n", file=sys.stderr)


def main():
    parser = argparse.ArgumentParser(description='`autojsoncxx` code generator '
                                                 '(visit https://github.com/netheril96/autojsoncxx for details)')

    parser.add_argument('-c', '--check', help='check the type names specified; requires `parsimonious` to be installed',
                        action='store_true', default=False)
    parser.add_argument('-i', '--input', help='input name for the definition file for classes', required=True)
    parser.add_argument('-o', '--output', help='output name for the header file', default=None)
    parser.add_argument('--template', help='location of the template file', default=None)
    args = parser.parse_args()

    if args.check and not parsimonious:
        print("Unable to import module `parsimonious`", "Type checks disabled", "", sep='\n', file=sys.stderr)
        args.check = False

    if args.template is None:
        if getattr(sys, 'frozen', False):
            executable_dir = os.path.dirname(sys.executable)
        else:
            executable_dir = os.path.dirname(os.path.abspath(__file__))
        args.template = os.path.join(executable_dir, 'code_template')

    with open(args.template) as f:
        template = f.read()

    cache = {'bool', 'char', 'int', 'unsigned int', 'unsigned', 'long long', 'long long int',
             'unsigned long long', 'unsigned long long int', 'std::uint32_t', 'std::int32_t',
             'std::uint64_t', 'std::int64_t', 'uint32_t', 'int32_t', 'uint64_t', 'int64_t', 'std::nullptr_t',
             'std::size_t', 'size_t', 'std::ptrdiff_t', 'ptrdiff_t',
             'double', 'std::string', 'std::vector', 'std::deque', 'std::array',
             'boost::container::vector', 'boost::container::deque', 'boost::array',
             'std::shared_ptr', 'std::unique_ptr', 'boost::shared_ptr', 'boost::optional',
             'std::map', 'std::unordered_map', 'std::multimap', 'std::unordered_multimap',
             'boost::unordered_map', 'boost::unordered_multimap', 'std::tuple'}

    def process_file(output):
        with open(args.input) as f:
            raw_record = json.load(f)

        output.write('#pragma once\n\n')

        def output_class(class_record):
            class_info = ClassInfo(class_record)
            cache.add(class_info.qualified_name().lstrip(':'))

            if args.check:
                check_all_members(class_info, cache)
            output.write(build_class(template, class_info))

        if isinstance(raw_record, list):
            for r in raw_record:
                output_class(r)
        else:
            output_class(raw_record)

    if args.output is None:
        args.output = os.path.basename(args.input)
        args.output = os.path.splitext(args.output)[0] + '.hpp'

    with open(args.output, 'w') as f:
        process_file(f)


if __name__ == '__main__':
    main()
