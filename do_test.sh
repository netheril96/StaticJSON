#!/bin/sh

git submodule init
git submodule update
python3 autojsoncxx.py examples/userdef.json --out=test/userdef.hpp
c++ -O2 -Wall -Wextra -pedantic -std=c++11 -I./include -I./rapidjson/include -I./catch/single_include test/main.cpp test/test.cpp -o autojsoncxx_test
./autojsoncxx_test --success
rm autojsoncxx_test
