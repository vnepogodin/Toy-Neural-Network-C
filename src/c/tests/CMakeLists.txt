option(NN_Sanitizer "Build test suite with sanitizer" OFF)
option(NN_Valgrind "Execute test suite with Valgrind" OFF)

set(NN_TEST_DIR "\"${PROJECT_SOURCE_DIR}/src/c/tests/src/\"")

if(NN_Sanitizer)
    message(STATUS "Building test suite with sanitizer")
    if(NOT MSVC)
        set(CMAKE_C_FLAGS "-g -Og -fsanitize=address -fsanitize=undefined -fno-omit-frame-pointer")

        # Clang only
        if (CMAKE_C_COMPILER_ID MATCHES "Clang")
            set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fsanitize=integer -fsanitize=nullability -fno-sanitize-recover=all -fsanitize-recover=unsigned-integer-overflow")
        endif()
    endif()
endif()

if(NN_Analyzer)
    message(STATUS "Enable static analyzer")
    # GCC only
    if (CMAKE_C_COMPILER_ID MATCHES "GCC")
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -mindirect-branch=thunk -fstack-clash-protection -fstack-protector-strong -fanalyzer")
    endif()
endif()

if(NN_Valgrind)
    find_program(CMAKE_MEMORYCHECK_COMMAND valgrind)
    message(STATUS "Executing test suite with Valgrind (${CMAKE_MEMORYCHECK_COMMAND})")
    set(memcheck_command "${CMAKE_MEMORYCHECK_COMMAND} ${CMAKE_MEMORYCHECK_COMMAND_OPTIONS} --quiet --leak-check=full --show-leak-kinds=all --track-origins=yes --error-exitcode=1")
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
    if(WIN32)
        target_link_libraries(${testcase} PRIVATE ${VNEPOGODIN_NN_TARGET_NAME})
    else()
        target_link_libraries(${testcase} PRIVATE ${VNEPOGODIN_NN_TARGET_NAME} m)
    endif()

    add_test(NAME "${testcase}"
            COMMAND ${testcase} --no-skip
            WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
            )
    set_tests_properties("${testcase}" PROPERTIES LABELS "all")

    if(NN_Valgrind)
        add_test(NAME "${testcase}_valgrind"
                COMMAND ${memcheck_command} ${CMAKE_CURRENT_BINARY_DIR}/${testcase}
                WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
                )
        set_tests_properties("${testcase}_valgrind" PROPERTIES LABELS "valgrind")
    endif()
endforeach()
