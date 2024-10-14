cmake_minimum_required(VERSION 3.28)
project(encviz)

# Required Libraries
include(FindPkgConfig)
pkg_check_modules(CAIRO cairo REQUIRED)
pkg_check_modules(GDAL gdal REQUIRED)
pkg_check_modules(MICROHTTPD libmicrohttpd REQUIRED)
pkg_check_modules(SQLITE3 sqlite3 REQUIRED)

# Header locations
include_directories(
  ${CAIRO_INCLUDE_DIRS}
  ${GDAL_INCLUDE_DIRS}
  ${MICROHTTPD_INCLUDE_DIRS}
  ${SQLITE3_INCLUDE_DIRS}
  ${PROJECT_SOURCE_DIR}/include
  )

# Collect all libs
set(ENCVIZ_LIBRARIES
  ${CAIRO_LIBRARIES}
  ${GDAL_LIBRARIES}
  ${MICROHTTPD_LIBRARIES}
  ${SQLITE3_LIBRARIES}
  )

# Organize build products
set(LIBRARY_OUTPUT_PATH "${CMAKE_CURRENT_BINARY_DIR}/lib")
set(EXECUTABLE_OUTPUT_PATH "${CMAKE_CURRENT_BINARY_DIR}/bin")

# Build source code
add_subdirectory(src)
