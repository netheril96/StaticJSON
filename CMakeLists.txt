cmake_minimum_required(VERSION 3.0)
project(StaticJSON)

option(STATICJSON_ENABLE_TEST "Enable building test for StaticJSON" ON)
option(STATICJSON_ASAN "Enable address sanitizer on non-MSVC" OFF)

set(CMAKE_CXX_STANDARD_REQUIRED 0)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE RelWithDebInfo)
endif()

if(MSVC)
  add_compile_definitions(_CRT_SECURE_NO_WARNINGS=1)
else()
  if(STATICJSON_ASAN)
    add_compile_options(-fsanitize=address)
    add_link_options(-fsanitize=address)
  endif()
endif()

find_path(RAPIDJSON_INCLUDE_DIR rapidjson/rapidjson.h)

set(SOURCE_FILES src/staticjson.cpp)
add_library(staticjson ${SOURCE_FILES})
set_property(TARGET staticjson PROPERTY CXX_STANDARD 11)
add_library(staticjson::staticjson ALIAS staticjson)
target_include_directories(
  staticjson
  PUBLIC $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>
         $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/autojsoncxx>
         $<BUILD_INTERFACE:${RAPIDJSON_INCLUDE_DIR}>
         $<INSTALL_INTERFACE:include>
  PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

if(STATICJSON_ENABLE_TEST)
  set(TARGET test_staticjson)
  file(GLOB SOURCES test/*.hpp test/*.cpp include/staticjson/*.hpp)
  add_executable(${TARGET} ${SOURCES})
  target_link_libraries(${TARGET} PRIVATE staticjson)
  set_property(TARGET ${TARGET} PROPERTY CXX_STANDARD 17)

  find_package(RapidJSON CONFIG REQUIRED)
  target_link_libraries(${TARGET} PRIVATE rapidjson)
  find_package(Catch2 CONFIG REQUIRED)
  target_link_libraries(${TARGET} PRIVATE Catch2::Catch2 Catch2::Catch2WithMain)

  enable_testing()
  add_test(
    NAME ${TARGET}
    COMMAND ${TARGET}
    WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/test)
endif()

include(GNUInstallDirs)

install(
  TARGETS staticjson
  EXPORT staticjson-targets
  ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
install(DIRECTORY ${CMAKE_SOURCE_DIR}/include/staticjson
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
install(
  EXPORT staticjson-targets
  FILE staticjson-targets.cmake
  NAMESPACE staticjson::
  DESTINATION ${CMAKE_INSTALL_DATADIR}/cmake/staticjson)

include(CMakePackageConfigHelpers)

configure_package_config_file(
  ${CMAKE_SOURCE_DIR}/cmake/staticjson-config.cmake.in
  ${CMAKE_BINARY_DIR}/cmake/staticjson-config.cmake
  INSTALL_DESTINATION ${CMAKE_INSTALL_DATADIR}/cmake/staticjson)

install(FILES ${CMAKE_BINARY_DIR}/cmake/staticjson-config.cmake
        DESTINATION ${CMAKE_INSTALL_DATADIR}/cmake/staticjson)
