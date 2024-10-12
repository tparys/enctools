cmake_minimum_required(VERSION 3.28)
project(encviz)

include(FindPkgConfig)
pkg_check_modules(MICROHTTPD libmicrohttpd REQUIRED)
pkg_check_modules(SQLITE3 sqlite3 REQUIRED)

# MicroHTTPD example app
include_directories(${MICROHTTPD_INCLUDE_DIRS})
add_executable(hellobrowser hellobrowser.c)
target_link_libraries(hellobrowser ${MICROHTTPD_LIBRARIES})

# MBTiles server
include_directories(${MICROHTTPD_INCLUDE_DIRS} ${SQLITE3_INCLUDE_DIRS})
add_executable(mbtiles_serve mbtiles_serve.cpp)
target_link_libraries(mbtiles_serve ${MICROHTTPD_LIBRARIES} ${SQLITE3_LIBRARIES})
