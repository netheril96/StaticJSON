#!/bin/sh
cd $(dirname $0)
GLOBIGNORE="include/staticjson/optional_support.hpp"
clang-format -i --style=File autojsoncxx/*.hpp include/staticjson/*.hpp src/*.cpp test/*.cpp test/myarray.hpp
