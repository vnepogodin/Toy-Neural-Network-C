cmake_minimum_required(VERSION 3.8)
project(Neural-Network)

option(LANG "which language do you prefer.\nON - C++\nOFF - C" OFF)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE debug)
endif()

if (NOT LANG)
    add_subdirectory(src/c)
    set(BUILD_TEST_FOLDER ${PROJECT_BINARY_DIR}/src/c/tests)
else()
    add_subdirectory(src/c++)
    set(BUILD_TEST_FOLDER ${PROJECT_BINARY_DIR}/src/c++/tests)
endif()

if (NOT MSVC)
add_custom_target(run
    COMMAND echo "init" && ./init && echo "nn_copy" && ./nn_copy && echo "nn_predict" && ./nn_predict && echo "nn_train" && ./nn_train && echo "json_serialize" && ./json_serialize && echo "json_deserialize" && ./json_deserialize
    WORKING_DIRECTORY ${BUILD_TEST_FOLDER})
endif()
