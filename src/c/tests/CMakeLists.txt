option(NN_Valgrind    "Execute test suite with Valgrind." OFF)
option(NN_FastTests   "Skip expensive/slow tests." OFF)

set(NN_TEST_DIR "\"${PROJECT_SOURCE_DIR}/src/c/tests/src/\"")

if(NN_Valgrind)
    find_program(CMAKE_MEMORYCHECK_COMMAND valgrind)
    message(STATUS "Executing test suite with Valgrind (${CMAKE_MEMORYCHECK_COMMAND})")
    set(memcheck_command "${CMAKE_MEMORYCHECK_COMMAND} ${CMAKE_MEMORYCHECK_COMMAND_OPTIONS} --error-exitcode=1 --leak-check=full")
    separate_arguments(memcheck_command)
endif()

#############################################################################
# one executable for each unit test file
#############################################################################

file(GLOB files src/unit-*.c)

foreach(file ${files})
    get_filename_component(file_basename ${file} NAME_WE)
    string(REGEX REPLACE "unit-([^$]+)" "test-\\1" testcase ${file_basename})

    add_executable(${testcase} ${file})
    target_compile_definitions(${testcase} PRIVATE NN_TEST_DIR=${NN_TEST_DIR} UNITY_OUTPUT_COLOR)
    target_compile_options(${testcase} PRIVATE
        $<$<C_COMPILER_ID:MSVC>:/EHsc;$<$<CONFIG:Release>:/Od>>
        $<$<NOT:$<C_COMPILER_ID:MSVC>>:-Wno-deprecated;-Wno-float-equal>
        $<$<C_COMPILER_ID:GNU>:-Wno-deprecated-declarations>
    )

    target_include_directories(${testcase} PRIVATE ${CMAKE_BINARY_DIR}/include thirdparty/Unity)
    if(WIN32)
        target_link_libraries(${testcase} PRIVATE ${VNEPOGODIN_NN_TARGET_NAME})
    else()
        target_link_libraries(${testcase} PRIVATE ${VNEPOGODIN_NN_TARGET_NAME} m)
    endif()

    if (NN_FastTests)
        add_test(NAME "${testcase}"
            COMMAND ${testcase}
            WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        )
    else()
        add_test(NAME "${testcase}"
            COMMAND ${testcase} --no-skip
            WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        )
    endif()

    set_tests_properties("${testcase}" PROPERTIES LABELS "all" FIXTURES_REQUIRED TEST_DATA)

    if(NN_Valgrind)
        add_test(NAME "${testcase}_valgrind"
            COMMAND ${memcheck_command} ${CMAKE_CURRENT_BINARY_DIR}/${testcase}
            WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        )
        set_tests_properties("${testcase}_valgrind" PROPERTIES LABELS "valgrind" FIXTURES_REQUIRED TEST_DATA)
    endif()
endforeach()
