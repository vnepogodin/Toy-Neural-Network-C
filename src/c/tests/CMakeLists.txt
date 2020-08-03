include_directories(PUBLIC ${CMAKE_SOURCE_DIR})

foreach(TESTNAME
    init
    nn_copy
    nn_predict
	nn_train
    json_serialize
    json_deserialize)

add_executable(${TESTNAME} ${TESTNAME}.c)
add_test(NAME ${TESTNAME} COMMAND ${PROJECT_SOURCE_DIR}/tests/${TESTNAME}.test)

# XXX using the non-target_ versions of these doesn't work :(
target_include_directories(
  ${TESTNAME}
  PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}
  )
target_link_libraries(
  ${TESTNAME}
  PRIVATE
    ${PROJECT_NAME}
  )

endforeach(TESTNAME)
