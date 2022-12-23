cmake_minimum_required(VERSION 3.0)
project(StaticJSON)

if (NOT MSVC)
    include(CheckCXXCompilerFlag)
    CHECK_CXX_COMPILER_FLAG(-std=c++17 CXX17_COMPILER)
    CHECK_CXX_COMPILER_FLAG(-std=c++14 CXX14_COMPILER)
    if (CXX17_COMPILER)
        add_compile_options(-std=c++17)
        add_definitions(-DSTATICJSON_OPTIONAL)
    elseif (CXX14_COMPILER)
        add_compile_options(-std=c++14)
        add_definitions(-DSTATICJSON_OPTIONAL)
    else ()
        add_compile_options(-std=c++11)
    endif ()
    add_compile_options(-fno-rtti -Wall -Wextra -pedantic -g)
endif ()

if (NOT CMAKE_BUILD_TYPE)
    SET(CMAKE_BUILD_TYPE RelWithDebInfo)
endif ()


find_path(RAPIDJSON_INCLUDE_DIR rapidjson/rapidjson.h)
include_directories(SYSTEM ${RAPIDJSON_INCLUDE_DIR})

include_directories(include autojsoncxx)
set(SOURCE_FILES src/staticjson.cpp)
add_library(staticjson ${SOURCE_FILES})

set(TARGET test_staticjson)
file(GLOB SOURCES test/*.hpp test/*.cpp include/staticjson/*.hpp)
add_executable(${TARGET} ${SOURCES})
target_link_libraries(${TARGET} staticjson)

enable_testing()
add_test(NAME ${TARGET} COMMAND ${TARGET} WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/test)
