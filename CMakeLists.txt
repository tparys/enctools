cmake_minimum_required(VERSION 3.16)
project(encviz)

# Need C++17
set(CMAKE_CXX_STANDARD 17)

# Use ccache if available
find_program(CCACHE_FOUND ccache)
if(CCACHE_FOUND)
  set_property(GLOBAL PROPERTY RULE_LAUNCH_COMPILE ccache)
endif()

set(CMAKE_BUILD_TYPE Release)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")

# Required Libraries
include(FindPkgConfig)
pkg_check_modules(CAIRO REQUIRED cairo)
pkg_check_modules(GDAL REQUIRED gdal)
pkg_check_modules(GTEST gtest_main gtest)
pkg_check_modules(MICROHTTPD REQUIRED libmicrohttpd)
pkg_check_modules(TINYXML2 REQUIRED tinyxml2)
find_package(CGAL REQUIRED)

# Pick GDAL memory driver to use
#  - Newer versions should use "Memory", "MEM" raises deprecation warnings
#  - Older versions should use "MEM", as "Memory" may not work at all
if (GDAL_VERSION VERSION_LESS "3.11")
  add_compile_definitions(GDAL_MEM_DRIVER="Memory")
else()
  add_compile_definitions(GDAL_MEM_DRIVER="MEM")
endif()

# Earlier versions of microhttpd don't define MHD_Result,
# and just use an int in it's place, so detect that ...
try_compile(MICROHTTP_HAS_MHD_RESULT
  ${CMAKE_BINARY_DIR}/compile_tests
  ${CMAKE_CURRENT_SOURCE_DIR}/cmake/microhttpd_mhd_result.c)
if (NOT MICROHTTP_HAS_MHD_RESULT)
  add_compile_definitions(MHD_Result=int)
endif()

# Header locations
include_directories(
  ${CAIRO_INCLUDE_DIRS}
  ${GDAL_INCLUDE_DIRS}
  ${MICROHTTPD_INCLUDE_DIRS}
  ${TINYXML2_INCLUDE_DIRS}
  ${PROJECT_SOURCE_DIR}/include
  )

# Organize build products
set(LIBRARY_OUTPUT_PATH "${CMAKE_CURRENT_BINARY_DIR}/lib")
set(EXECUTABLE_OUTPUT_PATH "${CMAKE_CURRENT_BINARY_DIR}/bin")

# Build source code
add_subdirectory(src)

# Build tests if able
if(GTEST_FOUND)
  enable_testing()
  add_subdirectory(test)
endif()
