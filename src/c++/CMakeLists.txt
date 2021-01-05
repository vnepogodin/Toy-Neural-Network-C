if(NOT MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DSIMDJSON_THREADS_ENABLED -ffast-math -flto")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS} ${CMAKE_CXX_FLAGS_RELEASE} -O3")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS} ${CMAKE_CXX_FLAGS_DEBUG} -Og")

    if ((${CMAKE_SYSTEM_NAME} MATCHES "Linux") OR (${CMAKE_SYSTEM_NAME} MATCHES "Darwin"))
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mavx2")
    endif()
endif()

if (NN_BuildTests)
    add_subdirectory(tests)
endif()

add_library(project_warnings INTERFACE)

include(cmake/CompilerWarnings.cmake)
set_project_warnings(project_warnings)
