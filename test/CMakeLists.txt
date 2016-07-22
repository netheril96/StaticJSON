CMAKE_MINIMUM_REQUIRED(VERSION 2.8)
PROJECT (test_staticjson)

if (NOT MSVC)
    SET(CMAKE_CXX_FLAGS "-std=c++11 -fno-rtti -Wall -Wextra -pedantic -g")
endif()

if (NOT CMAKE_BUILD_TYPE)
	SET (CMAKE_BUILD_TYPE Debug)
endif()

# Create a binary depending on generated header
INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}/../include)
SET(TARGET test_staticjson)
FILE (GLOB SOURCES ../include/staticjson/*.hpp ../src/*.cpp *.cpp)
ADD_EXECUTABLE(${TARGET} ${SOURCES})

# Perform testing
enable_testing()
add_test(NAME ${TARGET} COMMAND ${TARGET} WORKING_DIRECTORY ${PROJECT_SOURCE_DIR})
add_test(NAME test_schema COMMAND python test_schema.py WORKING_DIRECTORY ${PROJECT_SOURCE_DIR})
