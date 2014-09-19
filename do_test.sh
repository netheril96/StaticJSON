#!/bin/sh

git submodule init
git submodule update
python3 autojsoncxx.py examples/userdef.json --out=test/userdef.hpp

mkdir -p /tmp/26EB22B7-6CD9-4465-A5DE-04205274EE51/
c++ -O2 -Wall -Wextra -pedantic -std=c++11 -I./include -I./rapidjson/include -I./catch/single_include test/main.cpp test/test.cpp -o /tmp/26EB22B7-6CD9-4465-A5DE-04205274EE51/test_autojsoncxx
/tmp/26EB22B7-6CD9-4465-A5DE-04205274EE51/test_autojsoncxx --success
