
# MicroHTTPD example app
add_executable(hellobrowser hellobrowser.c)
target_link_libraries(hellobrowser encviz)

# GDAL test app
add_executable(gdal_test gdal_test.cpp)
target_link_libraries(gdal_test encviz)

# MBTiles reader
add_executable(mbtiles_reader mbtiles_reader.cpp)
target_link_libraries(mbtiles_reader encviz)

# MBTiles server
add_executable(mbtiles_serve mbtiles_serve.cpp)
target_link_libraries(mbtiles_serve encviz)

# Cairo 2D graphics
add_executable(cairo_stroke cairo_stroke.cpp)
target_link_libraries(cairo_stroke encviz)
