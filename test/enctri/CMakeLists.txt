add_executable(enctri_test
  triangle_face_test.cpp
  )
target_link_libraries(enctri_test enctri ${GTEST_LIBRARIES})
add_test(
  NAME enctri_test
  COMMAND "${EXECUTABLE_OUTPUT_PATH}/enctri_test"
  )
