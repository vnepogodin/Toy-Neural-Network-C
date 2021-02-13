cmake_minimum_required(VERSION 3.8)
project(NN_Benchmarks CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_BUILD_TYPE release)

# set compiler flags
if((CMAKE_CXX_COMPILER_ID MATCHES GNU) OR (CMAKE_CXX_COMPILER_ID MATCHES Clang))
   set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DSIMDJSON_THREADS_ENABLED -O3 -ffast-math -flto")
endif()

# configure Google Benchmarks
set(BENCHMARK_ENABLE_TESTING OFF CACHE INTERNAL "" FORCE)
add_subdirectory(thirdparty/benchmark)


# header directories
include_directories(thirdparty)
include_directories(${CMAKE_SOURCE_DIR}/../single_include/c++)

# benchmark binary
add_executable(${PROJECT_NAME} src/benchmarks.cpp)
target_link_libraries(${PROJECT_NAME} benchmark ${CMAKE_THREAD_LIBS_INIT})
target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_BINARY_DIR}/include)
