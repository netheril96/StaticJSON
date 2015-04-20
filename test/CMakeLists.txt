CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

SET(CMAKE_CXX_FLAGS "-std=c++0x -O2 -Wall -Wextra -pedantic -g")

# Generate header
SET(PYTHON_INTERP python)
SET(GENERATOR_SCRIPT ../autojsoncxx.py)
SET(DEFINITION_FILE ../examples/userdef.json)
SET(GENERATED_HPP userdef.hpp)
SET(CODE_TEMPLATE ../code_template)

EXECUTE_PROCESS(${PYTHON_INTERP} ${GENERATOR_SCRIPT} --check 
                --output=${GENERATED_HPP} --input=${DEFINITION_FILE})

# Create a binary depending on generated header
INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/include 
                    ${CMAKE_SOURCE_DIR}/rapidjson/include
                    ${CMAKE_SOURCE_DIR}/catch/single_include)
SET(TARGET autojsoncxx_catch)
SET(SOURCES main.cpp test.cpp)
ADD_EXECUTABLE(${TARGET} ${SOURCES})

ADD_TEST(${TARGET} --success)
