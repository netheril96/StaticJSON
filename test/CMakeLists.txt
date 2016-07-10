CMAKE_MINIMUM_REQUIRED(VERSION 2.8)
PROJECT (test_staticjson)

if (NOT MSVC)
    SET(CMAKE_CXX_FLAGS "-std=c++11 -fno-rtti -Wall -Wextra -pedantic -g")
endif()

if (NOT CMAKE_BUILD_TYPE)
	SET (CMAKE_BUILD_TYPE Release)
endif()

# Create a binary depending on generated header
INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}/../include)
SET(TARGET test_staticjson)
FILE (GLOB SOURCES ../include/staticjson/*.hpp ../src/*.cpp *.cpp)
ADD_EXECUTABLE(${TARGET} ${SOURCES})

# Perform testing
ADD_TEST(NAME ${TARGET} COMMAND ${TARGET})
