
# MicroHTTPD example app
add_executable(hellobrowser hellobrowser.c)
target_link_libraries(hellobrowser encviz)

# GDAL test app
add_executable(gdal_test gdal_test.cpp)
target_link_libraries(gdal_test encviz)

# Cairo 2D graphics
add_executable(cairo_stroke cairo_stroke.cpp)
target_link_libraries(cairo_stroke encviz)

# ENC index test
add_executable(enc_tile_render enc_tile_render.cpp)
target_link_libraries(enc_tile_render encviz)

# ENC WMTS tile server
add_executable(enc_tile_server enc_tile_server.cpp)
target_link_libraries(enc_tile_server encviz)
