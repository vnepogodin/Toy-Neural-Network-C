cmake_minimum_required(VERSION 3.8)
project(Neural-Network-CPP CXX)

set(CMAKE_CXX_STANDARD 17)

if(NOT MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -ffast-math")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O2")
endif()

add_subdirectory(tests)

add_library(project_warnings INTERFACE)

include(cmake/CompilerWarnings.cmake)
set_project_warnings(project_warnings)

set(NN_HEADERS
    #        ../../include/c++/third_party/json/json.hpp
        ../../include/c++/Matrix.hpp
        ../../include/c++/nn.hpp)

set(NN_SOURCES
        Matrix.cpp
        nn.cpp)

add_library(${PROJECT_NAME}
        ${NN_SOURCES}
        ${NN_HEADERS})

set_target_properties(${PROJECT_NAME} PROPERTIES
        VERSION 0.0.5
        SOVERSION 0)

list(APPEND CMAKE_TARGETS ${PROJECT_NAME})
