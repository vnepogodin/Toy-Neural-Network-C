option(NN_Valgrind    "Execute test suite with Valgrind." OFF)
option(NN_FastTests   "Skip expensive/slow tests." OFF)

set(NN_TEST_DIR "\"${PROJECT_SOURCE_DIR}/src/c++/tests/src/\"")

if(NN_Valgrind)
    find_program(CMAKE_MEMORYCHECK_COMMAND valgrind)
    message(STATUS "Executing test suite with Valgrind (${CMAKE_MEMORYCHECK_COMMAND})")
    set(memcheck_command "${CMAKE_MEMORYCHECK_COMMAND} ${CMAKE_MEMORYCHECK_COMMAND_OPTIONS} --error-exitcode=1 --leak-check=full")
    separate_arguments(memcheck_command)
endif()

#############################################################################
# doctest library with the main function to speed up build
#############################################################################

add_library(doctest_main OBJECT src/unit.cpp)
set_target_properties(doctest_main PROPERTIES
    COMPILE_DEFINITIONS "$<$<CXX_COMPILER_ID:MSVC>:_SCL_SECURE_NO_WARNINGS>"
    COMPILE_OPTIONS "$<$<CXX_COMPILER_ID:MSVC>:/EHsc;$<$<CONFIG:Release>:/Od>>"
)
target_compile_features(doctest_main PUBLIC cxx_std_17)
target_include_directories(doctest_main PRIVATE "thirdparty/doctest")

if(MSVC)
    # https://github.com/nlohmann/json/issues/1114
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /bigobj")
endif()

#############################################################################
# one executable for each unit test file
#############################################################################

file(GLOB files src/unit-*.cpp)

foreach(file ${files})
    get_filename_component(file_basename ${file} NAME_WE)
    string(REGEX REPLACE "unit-([^$]+)" "test-\\1" testcase ${file_basename})

    add_executable(${testcase} $<TARGET_OBJECTS:doctest_main> ${file})
    target_compile_definitions(${testcase} PRIVATE DOCTEST_CONFIG_SUPER_FAST_ASSERTS)
    target_compile_definitions(${testcase} PRIVATE NN_TEST_DIR=${NN_TEST_DIR})
    target_compile_options(${testcase} PRIVATE
        $<$<CXX_COMPILER_ID:MSVC>:/EHsc;$<$<CONFIG:Release>:/Od>>
        $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wno-deprecated;-Wno-float-equal>
        $<$<CXX_COMPILER_ID:GNU>:-Wno-deprecated-declarations>
    )
    target_include_directories(${testcase} PRIVATE ${CMAKE_BINARY_DIR}/include thirdparty/doctest)
if(NN_SIMD)
    target_link_libraries(${testcase} PRIVATE ${VNEPOGODIN_NN_TARGET_NAME} Vc)
else()
    target_link_libraries(${testcase} PRIVATE ${VNEPOGODIN_NN_TARGET_NAME})
endif()

    if (NN_FastTests)
        add_test(NAME "${testcase}"
            COMMAND ${testcase} ${DOCTEST_TEST_FILTER}
            WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        )
    else()
        add_test(NAME "${testcase}"
            COMMAND ${testcase} ${DOCTEST_TEST_FILTER} --no-skip
            WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        )
    endif()

    set_tests_properties("${testcase}" PROPERTIES LABELS "all" FIXTURES_REQUIRED TEST_DATA)

    if(NN_Valgrind)
        add_test(NAME "${testcase}_valgrind"
            COMMAND ${memcheck_command} ${CMAKE_CURRENT_BINARY_DIR}/${testcase} ${DOCTEST_TEST_FILTER}
            WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        )
        set_tests_properties("${testcase}_valgrind" PROPERTIES LABELS "valgrind" FIXTURES_REQUIRED TEST_DATA)
    endif()
endforeach()
