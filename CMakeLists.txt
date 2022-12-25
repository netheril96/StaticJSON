cmake_minimum_required(VERSION 3.0)
project(StaticJSON)

set(CMAKE_CXX_STANDARD_REQUIRED 0)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE RelWithDebInfo)
endif()

find_path(RAPIDJSON_INCLUDE_DIR rapidjson/rapidjson.h)
include_directories(SYSTEM ${RAPIDJSON_INCLUDE_DIR})

include_directories(include autojsoncxx)
set(SOURCE_FILES src/staticjson.cpp)
add_library(staticjson ${SOURCE_FILES})
set_property(TARGET staticjson PROPERTY CXX_STANDARD 11)

set(TARGET test_staticjson)
file(GLOB SOURCES test/*.hpp test/*.cpp include/staticjson/*.hpp)
add_executable(${TARGET} ${SOURCES})
target_link_libraries(${TARGET} staticjson)
set_property(TARGET ${TARGET} PROPERTY CXX_STANDARD 17)

enable_testing()
add_test(
  NAME ${TARGET}
  COMMAND ${TARGET}
  WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/test)
