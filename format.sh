#!/bin/sh
cd $(dirname $0)
GLOBIGNORE="optional_support.hpp"
clang-format -i --style=File autojsoncxx/*.hpp include/staticjson/*.hpp src/*.cpp test/*.cpp
