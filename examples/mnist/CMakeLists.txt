cmake_minimum_required(VERSION 3.12)
project(mnist LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -flto")

include_directories(${CMAKE_SOURCE_DIR}/../../single_include/c++)
add_executable(mnist main.cpp mnist.cpp data_view/data_view.cc sdl_texture.hpp)
target_link_libraries(mnist fmt SDL2 SDL2_ttf)
