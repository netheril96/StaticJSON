CMAKE_MINIMUM_REQUIRED(VERSION 2.8)
PROJECT (bench_staticjson)

if (NOT MSVC)
    SET(CMAKE_CXX_FLAGS "-std=c++11 -Wall -Wextra -pedantic -g")
endif()

if (NOT CMAKE_BUILD_TYPE)
	SET (CMAKE_BUILD_TYPE Release)
endif()

INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}/../include)
INCLUDE_DIRECTORIES(.)
SET(TARGET bench_staticjson)
FILE (GLOB SOURCES ../include/staticjson/*.hpp ../src/*.cpp *.cpp)
ADD_EXECUTABLE(${TARGET} ${SOURCES})

