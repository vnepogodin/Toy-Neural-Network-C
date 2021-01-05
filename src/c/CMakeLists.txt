if(NOT MSVC)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -ffast-math -Wall -Wextra -Wshadow -Wcast-align -Wunused -Wconversion -Wsign-conversion -Wformat=2")
    set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -O2")
    set(CMAKE_C_FLAGS_DEBUG   "${CMAKE_C_FLAGS_DEBUG}   -Og")
endif()

if (NN_BuildTests)
    add_subdirectory(tests)
endif()
