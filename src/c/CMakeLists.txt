cmake_minimum_required(VERSION 3.8)
project(Neural-Network-C C)

set(CMAKE_C_STANDARD 99)

if(NOT MSVC)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -ffast-math -Wall -Wextra -Wshadow -Wcast-align -Wunused -Wconversion -Wsign-conversion -Wformat=2")
    set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -O2")
    set(CMAKE_C_FLAGS_DEBUG   "${CMAKE_C_FLAGS_DEBUG}   -Og")
endif()
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

target_link_libraries(${PROJECT_NAME} PUBLIC m)

list(APPEND CMAKE_TARGETS ${PROJECT_NAME})
