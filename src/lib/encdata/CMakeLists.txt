add_library(encdata
  enc_dataset.cpp
  )
target_link_libraries(encdata
  ${GDAL_LIBRARIES}
  )
