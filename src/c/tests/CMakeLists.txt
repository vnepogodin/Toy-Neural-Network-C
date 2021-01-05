option(NN_Sanitizer "Build test suite with Clang sanitizer" OFF)
option(NN_Valgrind "Execute test suite with Valgrind" OFF)

if(NN_Sanitizer)
    message(STATUS "Building test suite with Clang sanitizer")
    if(NOT MSVC)
        set(CMAKE_C_FLAGS "-g -O0 -fsanitize=address -fsanitize=undefined -fsanitize=integer -fsanitize=nullability -fno-omit-frame-pointer -fno-sanitize-recover=all -fsanitize-recover=unsigned-integer-overflow")
    endif()
endif()
#include_directories(PUBLIC ${CMAKE_SOURCE_DIR})

#############################################################################
# one executable for each unit test file
#############################################################################

set(files
    init
    nn_copy
    nn_predict
    nn_train
    json_serialize
    json_deserialize)

foreach(file ${files})
    add_executable(${file} ${file}.c)
    target_compile_options(${file} PRIVATE
        $<$<CXX_COMPILER_ID:MSVC>:/EHsc;$<$<CONFIG:Release>:/Od>>
        $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wno-float-equal>
    )
    target_include_directories(${file} PRIVATE ${CMAKE_BINARY_DIR}/include)
    target_link_libraries(${file} PRIVATE ${VNEPOGODIN_NN_TARGET_NAME} m)
endforeach()

add_executable(nn_unit EXCLUDE_FROM_ALL ${files})
target_compile_options(nn_unit PRIVATE
    $<$<CXX_COMPILER_ID:MSVC>:/EHsc;$<$<CONFIG:Release>:/Od>>
    $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wno-float-equal>
)
target_include_directories(nn_unit PRIVATE ${CMAKE_BINARY_DIR}/include)
target_link_libraries(nn_unit ${VNEPOGODIN_NN_TARGET_NAME} m)
