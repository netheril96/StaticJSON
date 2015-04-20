CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

SET(CMAKE_CXX_FLAGS "-std=c++0x -O2 -Wall -Wextra -pedantic -g")

# Generate header
IF(UNIX OR CYGWIN)
    SET(PYTHON_INTERP /usr/bin/python)
ELSEIF (WIN32)
    SET(PYTHON_INTERP python)
ENDIF()
SET(GENERATOR_SCRIPT ${CMAKE_SOURCE_DIR}/autojsoncxx.py)
SET(DEFINITION_FILE ${CMAKE_SOURCE_DIR}/examples/userdef.json)
SET(GENERATED_HPP ${CMAKE_SOURCE_DIR}/test/userdef.hpp)
SET(CODE_TEMPLATE ${CMAKE_SOURCE_DIR}/code_template)

EXEC_PROGRAM("${PYTHON_INTERP} ${GENERATOR_SCRIPT} --check --output=${GENERATED_HPP} --input=${DEFINITION_FILE}")

# Create a binary depending on generated header
INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/include 
                    ${CMAKE_SOURCE_DIR}/rapidjson/include
                    ${CMAKE_SOURCE_DIR}/catch/single_include)
SET(TARGET autojsoncxx_catch)
SET(SOURCES main.cpp test.cpp)
ADD_EXECUTABLE(${TARGET} ${SOURCES})

# Perform testing
ADD_TEST(NAME ${TARGET} COMMAND ${TARGET} --success)
