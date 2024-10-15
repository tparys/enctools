add_library(encviz
  web_mercator.cpp
  )
target_link_libraries(encviz
  ${CAIRO_LIBRARIES}
  ${GDAL_LIBRARIES}
  ${MICROHTTPD_LIBRARIES}
  ${SQLITE3_LIBRARIES}
  )


