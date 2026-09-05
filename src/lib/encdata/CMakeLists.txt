add_library(encdata
  config.cpp
  enc_dataset.cpp
  )
target_link_libraries(encdata
  ${GDAL_LIBRARIES}
  ${JSONCPP_LIBRARIES}
  )
