option(NN_Sanitizer "Build test suite with sanitizer" OFF)
option(NN_Analyzer "Enable static analyzer" OFF)
option(NN_Valgrind "Execute test suite with Valgrind" OFF)
option(NN_Coverage "Build test suite with coverage information" OFF)

set(NN_TEST_DIR "\"${PROJECT_SOURCE_DIR}/src/c++/tests/src/\"")

if(NN_Sanitizer)
    message(STATUS "Building test suite with sanitizer")
    if(NOT MSVC)
        set(CMAKE_CXX_FLAGS "-g -Og -fsanitize=address -fsanitize=undefined -fno-omit-frame-pointer")

        # Clang only
        if (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
            set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=integer -fsanitize=nullability -fno-sanitize-recover=all -fsanitize-recover=unsigned-integer-overflow")
        endif()
    endif()
endif()

if(NN_Analyzer)
    message(STATUS "Enable static analyzer")
    # GCC only
    if (CMAKE_CXX_COMPILER_ID MATCHES "GCC")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mindirect-branch=thunk -fstack-clash-protection -fstack-protector-strong -fanalyzer")
    endif()
endif()

if(NN_Valgrind)
    find_program(CMAKE_MEMORYCHECK_COMMAND valgrind)
    message(STATUS "Executing test suite with Valgrind (${CMAKE_MEMORYCHECK_COMMAND})")
    set(memcheck_command "${CMAKE_MEMORYCHECK_COMMAND} ${CMAKE_MEMORYCHECK_COMMAND_OPTIONS} --error-exitcode=1 --leak-check=full")
    separate_arguments(memcheck_command)
endif()

if(NN_Coverage)
    message(STATUS "Building test suite with coverage information")

    # from https://github.com/RWTH-HPC/CMake-codecov/blob/master/cmake/FindGcov.cmake
    get_filename_component(COMPILER_PATH "${CMAKE_CXX_COMPILER}" PATH)
    string(REGEX MATCH "^[0-9]+" GCC_VERSION "${CMAKE_CXX_COMPILER_VERSION}")
    find_program(GCOV_BIN NAMES gcov-${GCC_VERSION} gcov HINTS ${COMPILER_PATH})

    # collect all source files from the chosen include dir
    file(GLOB SOURCE_FILES ${NN_BUILD_LANG_SOURCES_DIR}/*.cpp)

    # add target to collect coverage information and generate HTML file
    # (filter script from https://stackoverflow.com/a/43726240/266378)
    add_custom_target(lcov_html
        COMMAND lcov --directory ${CMAKE_BINARY_DIR} --capture --output-file nn.info --rc lcov_branch_coverage=1
        COMMAND lcov -e nn.info ${SOURCE_FILES} --output-file nn.info.filtered --gcov-tool ${GCOV_BIN} --rc lcov_branch_coverage=1
        COMMAND ${CMAKE_SOURCE_DIR}/tests/thirdparty/imapdl/filterbr.py nn.info.filtered > nn.info.filtered.noexcept
        COMMAND genhtml --title "Neural Network C++ library" --legend --demangle-cpp --output-directory html --show-details --branch-coverage nn.info.filtered.noexcept
        COMMENT "Generating HTML report test/html/index.html"
    )
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

set(files
    src/unit-init.cpp
    src/unit-nn_copy.cpp
    src/unit-nn_predict.cpp
    src/unit-nn_train.cpp
    src/unit-json_serialize.cpp
    src/unit-json_deserialize.cpp)

foreach(file ${files})
    get_filename_component(file_basename ${file} NAME_WE)
    string(REGEX REPLACE "unit-([^$]+)" "test-\\1" testcase ${file_basename})

    add_executable(${testcase} $<TARGET_OBJECTS:doctest_main> ${file})
    target_compile_definitions(${testcase} PRIVATE DOCTEST_CONFIG_SUPER_FAST_ASSERTS)
    target_compile_definitions(${testcase} PRIVATE NN_TEST_DIR=${NN_TEST_DIR})
    target_compile_options(${testcase} PRIVATE
        $<$<CXX_COMPILER_ID:MSVC>:/EHsc;$<$<CONFIG:Release>:/Od>>
        $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wno-float-equal>
    )
    target_include_directories(${testcase} PRIVATE ${CMAKE_BINARY_DIR}/include thirdparty/doctest)
    target_link_libraries(${testcase} PRIVATE ${VNEPOGODIN_NN_TARGET_NAME})

    if (NN_Coverage)
        target_compile_options(${testcase} PRIVATE --coverage)
        target_link_libraries(${testcase} PRIVATE --coverage)
    endif()

    add_test(NAME "${testcase}"
            COMMAND ${testcase} ${DOCTEST_TEST_FILTER} --no-skip
            WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
            )
    set_tests_properties("${testcase}" PROPERTIES LABELS "all")

    if(NN_Valgrind)
        add_test(NAME "${testcase}_valgrind"
                COMMAND ${memcheck_command} ${CMAKE_CURRENT_BINARY_DIR}/${testcase} ${DOCTEST_TEST_FILTER}
                WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
                )
        set_tests_properties("${testcase}_valgrind" PROPERTIES LABELS "valgrind")
    endif()
endforeach()
