option(NN_Sanitizer "Build test suite with sanitizer" OFF)
option(NN_Valgrind "Execute test suite with Valgrind" OFF)

set(NN_TEST_DIR "\"${PROJECT_SOURCE_DIR}/src/c/tests/src/\"")

if(NN_Sanitizer)
    message(STATUS "Building test suite with sanitizer")
    if(NOT MSVC)
        set(CMAKE_CXX_FLAGS "-g -Og -fsanitize=address -fsanitize=undefined -fno-omit-frame-pointer")
    endif()
endif()

if(NN_Valgrind)
    find_program(CMAKE_MEMORYCHECK_COMMAND valgrind)
    message(STATUS "Executing test suite with Valgrind (${CMAKE_MEMORYCHECK_COMMAND})")
    set(memcheck_command "${CMAKE_MEMORYCHECK_COMMAND} ${CMAKE_MEMORYCHECK_COMMAND_OPTIONS} --error-exitcode=1 --leak-check=full")
    separate_arguments(memcheck_command)
endif()

#############################################################################
# one executable for each unit test file
#############################################################################

set(files
    src/unit-init.c
    src/unit-nn_copy.c
    src/unit-nn_predict.c
    src/unit-nn_train.c
    src/unit-json_serialize.c
    src/unit-json_deserialize.c)

foreach(file ${files})
    get_filename_component(file_basename ${file} NAME_WE)
    string(REGEX REPLACE "unit-([^$]+)" "test-\\1" testcase ${file_basename})

    add_executable(${testcase} ${file})
    target_compile_options(${testcase} PRIVATE
        $<$<CXX_COMPILER_ID:MSVC>:/EHsc;$<$<CONFIG:Release>:/Od>>
        $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wno-float-equal>
    )
    target_include_directories(${testcase} PRIVATE ${CMAKE_BINARY_DIR}/include)
    target_link_libraries(${testcase} PRIVATE ${VNEPOGODIN_NN_TARGET_NAME} m)

    if (JSON_Coverage)
        target_compile_options(${testcase} PRIVATE --coverage)
        target_link_libraries(${testcase} PRIVATE --coverage)
    endif()

    add_test(NAME "${testcase}"
            COMMAND ${testcase} --no-skip
            WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
            )
    set_tests_properties("${testcase}" PROPERTIES LABELS "all")

    if(JSON_Valgrind)
        add_test(NAME "${testcase}_valgrind"
                COMMAND ${memcheck_command} ${CMAKE_CURRENT_BINARY_DIR}/${testcase}
                WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
                )
        set_tests_properties("${testcase}_valgrind" PROPERTIES LABELS "valgrind")
    endif()
endforeach()
