add_executable(encviz_test
  web_mercator_test.cpp
  )
target_link_libraries(encviz_test encviz ${GTEST_LIBRARIES})
add_test(
  NAME encviz_test
  COMMAND "${EXECUTABLE_OUTPUT_PATH}/encviz_test"
  )
