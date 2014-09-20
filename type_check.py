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


import parsimonious

grammar = parsimonious.Grammar(r'''
    type = (space simple_type space "<" space type_list space ">" space) / ( space simple_type space )
    type_list = (type space "," space type_list) / type
    simple_type = spaced_type / ("::"? identifier ("::" identifier)*)
    spaced_type = "unsigned long long int" / "unsigned long long" / "unsigned int" / "long long int" / "long long"
    identifier = ~"[A-Za-z_][A-Za-z_0-9]*"
    space = ~"[ \t]*"
''')


def extract_simple_type(node):
    if node.expr_name == 'simple_type':
        yield node.text

    for sub_node in node.children:
        for value in extract_simple_type(sub_node):
            yield value


def check_type_name(name, cache):
    '''
    :param name: the full name of the type to check
    :param cache: the names that has been encountered so far; updated after function returns
    :return: None if error in parsing the name; otherwise a list of unknown types
    '''
    try:
        node = grammar.parse(name)
        simple_types = set(extract_simple_type(node))
        unknowns = simple_types - cache
        cache.update(simple_types)
        return unknowns

    except parsimonious.ParseError:
        return None


print(check_type_name("std::vector<std::basic_string<unsigned char>, n>", {"std::vector"}))