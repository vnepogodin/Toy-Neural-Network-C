cmake_minimum_required(VERSION 3.16)
project(Neural-Network-C C)

set(CMAKE_C_STANDARD 90)
set(CMAKE_C_FLAGS "-ffast-math -Wall -Wextra -Wshadow -Wcast-align -Wunused -Wconversion -Wsign-conversion -Wformat=2")

add_subdirectory(tests)

set(NN_HEADERS
        ../../include/c/matrix.h
        ../../include/c/nn.h)

set(NN_SOURCES
        ../../include/c/third_party/json-c/json_tokener.c
        ../../include/c/third_party/json-c/linkhash.c
        ../../include/c/third_party/json-c/printbuf.c
        ../../include/c/third_party/json-c/arraylist.c
        ../../include/c/third_party/json-c/json_object.c
        matrix.c
        nn.c)

add_library(${PROJECT_NAME}
        ${NN_SOURCES}
        ${NN_HEADERS})
set_target_properties(${PROJECT_NAME} PROPERTIES
        VERSION 0.0.5
        SOVERSION 0)
list(APPEND CMAKE_TARGETS ${PROJECT_NAME})

if (BUILD_STATIC_LIBS AND BUILD_SHARED_LIBS)
    set(STATIC_LIB ${PROJECT_NAME}-static)
    add_library(${STATIC_LIB} STATIC
            ${NN_SOURCES}
            ${NN_HEADERS})

    # rename the static library
    if (NOT MSVC)
        set_target_properties(${STATIC_LIB} PROPERTIES
                OUTPUT_NAME ${PROJECT_NAME})
    endif()
    list(APPEND CMAKE_TARGETS ${STATIC_LIB})
endif ()

if (NOT MSVC)
add_custom_target(run
    COMMAND echo "init" && ${PROJECT_BINARY_DIR}/tests/init && echo "nn_copy" && ${PROJECT_BINARY_DIR}/tests/nn_copy && echo "nn_predict" && ${PROJECT_BINARY_DIR}/tests/nn_predict && echo "nn_train" && ${PROJECT_BINARY_DIR}/tests/nn_train && echo "json_serialize" && ${PROJECT_BINARY_DIR}/tests/json_serialize && echo "json_deserialize" && ${PROJECT_BINARY_DIR}/tests/json_deserialize
  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)
endif()
