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

"""
This script allows easier transition from StaticJSON's precursor `autojsoncxx`.

Generates compatible C++ header files using StaticJSON's library.
"""

from __future__ import unicode_literals
from __future__ import print_function

import re
import argparse
import os
import string
import sys
import io
from numbers import Number


# Python 2/3 compatibility layer
is_python2 = sys.version_info.major == 2
if is_python2:
    str = unicode

# simplejson has the same interface as the standard json module, but with better error messages
try:
    import simplejson as json
except ImportError:
    import json


# base class for all custom exceptions in this unit
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


NOESCAPE_CHARACTERS = bytes(string.digits + string.ascii_letters + ' ', encoding='utf8')


def cstring_literal(byte_string):
    """convert arbitrary byte sequence into a C++ string literal by escaping every character"""
    if all(c in NOESCAPE_CHARACTERS for c in byte_string):
        return '"' + byte_string.decode() + '"'
    return '"' + ''.join('\\x{:02x}'.format(ord(char)) for char in byte_string) + '"'


def check_identifier(identifier):
    if not re.match(r'^[A-Za-z_]\w*$', identifier):
        raise InvalidIdentifier(identifier)


class ClassInfo(object):
    accept_options = {"name", "namespace", "parse_mode", "members", "constructor_code", "comment", "no_duplicates"}

    def __init__(self, record):
        self._name = record['name']
        self._members = [MemberInfo(r) for r in record['members']]
        self._strict = record.get('parse_mode', '') == 'strict'
        self._namespace = record.get("namespace", None)
        self._constructor_code = record.get("constructor_code", "")
        self._no_duplicates = record.get("no_duplicates", False)

        check_identifier(self._name)

        if self._namespace is not None and not re.match(r'^(?:::)?[A-Za-z_]\w*(?:::[A-Za-z_]\w*)*$', self._namespace):
            raise InvalidNamespace(self._namespace)

        for op in record:
            if op not in ClassInfo.accept_options:
                raise UnrecognizedOption(op)

    @property
    def name(self):
        return self._name

    @property
    def qualified_name(self):
        if self.namespace is None:
            return '::' + self.name
        if self.namespace.startswith('::'):
            return self.namespace + '::' + self.name
        return '::' + self.namespace + '::' + self.name

    @property
    def members(self):
        return self._members

    @property
    def strict_parsing(self):
        return self._strict

    @property
    def namespace(self):
        return self._namespace

    @property
    def constructor_code(self):
        return self._constructor_code

    @property
    def no_duplicates(self):
        return self._no_duplicates


class ClassDefinitionCodeGenerator(object):
    def __init__(self, class_info):
        self._class_info = class_info

    @property
    def class_info(self):
        return self._class_info

    def member_declarations(self):
        return '\n'.join(m.type_name + ' ' + m.variable_name + ';' for m in self.class_info.members)

    def initializer_list(self):
        return ', '.join('{0}({1})'.format(m.variable_name, m.constructor_args) for m in self.class_info.members)

    def constructor(self):
        return 'explicit {name}():{init} {{ {code} }}\n'.format(name=self.class_info.name,
                                                                init=self.initializer_list(),
                                                                code=self.class_info.constructor_code)

    def staticjson_init(self):
        class_flags = 'staticjson::Flags::Default'
        if not self.class_info.no_duplicates:
            class_flags += ' | staticjson::Flags::AllowDuplicateKey'
        if self.class_info.strict_parsing:
            class_flags += ' | staticjson::Flags::DisallowUnknownKey'

        return """
            void staticjson_init(staticjson::ObjectHandler* h) {{
                {member_flag_settings}
                h->set_flags({class_flags});
             }}
         """.format(
            class_flags=class_flags,
            member_flag_settings=''.join(m.add_property_statement('h') for m in self.class_info.members)
        )

    def class_definition(self):
        class_def = 'struct {name} {{\n {declarations}\n\n{constructor}\n{staticjson_init}\n \n}};' \
            .format(name=self.class_info.name, declarations=self.member_declarations(),
                    constructor=self.constructor(),
                    staticjson_init=self.staticjson_init())

        if self.class_info.namespace is not None:
            for space in reversed(self.class_info.namespace.split('::')):
                if space:
                    class_def = 'namespace {} {{ {} }}\n'.format(space, class_def)
        return class_def


class MemberInfo(object):
    accept_options = {'default', 'required', 'json_key', 'comment'}

    def __init__(self, record):
        self._record = record

        if '*' in self.type_name or '&' in self.type_name:
            raise UnsupportedTypeError(self.type_name)

        check_identifier(self.variable_name)

        if len(record) > 3:
            raise UnrecognizedOption(record[3:])

        if len(record) == 3:
            for op in record[2]:
                if op not in MemberInfo.accept_options:
                    raise UnrecognizedOption(op)

    @property
    def type_name(self):
        return self._record[0]

    @property
    def variable_name(self):
        return self._record[1]

    @property
    def json_key(self):
        try:
            return self._record[2]['json_key'].encode('utf-8')
        except (IndexError, KeyError):
            return self.variable_name.encode('utf-8')

    @property
    def is_required(self):
        try:
            return self._record[2]['required']
        except (IndexError, KeyError):
            return False

    @property
    def default(self):
        try:
            return self._record[2]['default']
        except (IndexError, KeyError):
            return None

    @property
    def constructor_args(self):
        return MemberInfo.cpp_repr(self.default)

    def add_property_statement(self, handler_name):
        return '{}->add_property({}, &this->{}, {});\n'.format(
            handler_name, cstring_literal(self.json_key), self.variable_name,
            'staticjson::Flags::Default' if self.is_required else 'staticjson::Flags::Optional')

    @staticmethod
    def cpp_repr(args):
        if args is None:
            return ''
        elif args is True:
            return 'true'
        elif args is False:
            return 'false'
        elif isinstance(args, str):
            return cstring_literal(args.encode('utf-8'))
        elif isinstance(args, Number):
            return str(args)
        elif isinstance(args, bytes):
            return cstring_literal(args)
        else:
            raise UnrecognizedOption("default=" + repr(args))


def read_utf8(filename):
    with io.open(filename, 'rt', encoding='utf-8') as f:
        text = f.read()
    if text.startswith(u'\ufeff'): # Skip BOM
        text = text[1:]
    return text


def main():
    parser = argparse.ArgumentParser(description='`autojsoncxx` code generator (compatibility mode)\n'
                                                 '(visit https://github.com/netheril96/StaticJSON for details)')

    parser.add_argument('-c', '--check', help='Compatibility flag; does nothing',
                        action='store_true', default=False)
    parser.add_argument('-i', '--input', help='input name for the definition file for classes', required=True)
    parser.add_argument('-o', '--output', help='output name for the header file', default=None)
    parser.add_argument('--template', help='Compatibility flag; does nothing', default=None)
    args = parser.parse_args()

    if args.output is None:
        args.output = os.path.basename(args.input)
        args.output = os.path.splitext(args.output)[0] + '.hpp'

    raw_record = json.loads(read_utf8(args.input))

    with io.open(args.output, 'w', encoding='utf-8') as output:
        output.write('#pragma once\n\n')

        def output_class(class_record):
            output.write(ClassDefinitionCodeGenerator(ClassInfo(class_record)).class_definition())
            output.write('\n\n')

        if isinstance(raw_record, list):
            for r in raw_record:
                output_class(r)
        else:
            output_class(raw_record)


if __name__ == '__main__':
    main()
