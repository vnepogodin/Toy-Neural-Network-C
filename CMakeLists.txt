cmake_minimum_required(VERSION 3.8)

##
## PROJECT
## name and version
##
project(vnepogodin_nn
        VERSION 0.5.0
        LANGUAGES CXX C)

##
## MAIN_PROJECT CHECK
## determine if vnepogodin_nn is built as a subproject (using add_subdirectory) or if it is the main project
##
set(MAIN_PROJECT OFF)
if (CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)
  set(MAIN_PROJECT ON)
endif()

if(NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_CONFIGURATION_TYPES Debug)
endif()


##
## INCLUDE
##
include(ExternalProject)
include(FindThreads)

find_package(Threads REQUIRED)

##
## OPTIONS
##
if (POLICY CMP0077)
    # Allow CMake 3.13+ to override options when using FetchContent / add_subdirectory.
    cmake_policy(SET CMP0077 NEW)
endif ()

option(NN_LANG "which language do you prefer.\nON - C++\nOFF - C" ON)
option(NN_BuildTests "Build the unit tests when BUILD_TESTING is enabled." ${MAIN_PROJECT})
option(NN_Install "Install CMake targets during install step." ${MAIN_PROJECT})
option(NN_MultipleHeaders "Use non-amalgamated version of the library." OFF)

##
## CONFIGURATION
##
include(GNUInstallDirs)

set(VNEPOGODIN_NN_TARGET_NAME                ${PROJECT_NAME})
set(VNEPOGODIN_NN_CONFIG_INSTALL_DIR         "${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}" CACHE INTERNAL "")
set(VNEPOGODIN_NN_INCLUDE_INSTALL_DIR        "${CMAKE_INSTALL_INCLUDEDIR}")
set(VNEPOGODIN_NN_LIB_INSTALL_DIR            "${CMAKE_INSTALL_LIBDIR}")
set(VNEPOGODIN_NN_TARGETS_EXPORT_NAME        "${PROJECT_NAME}Targets")
set(VNEPOGODIN_NN_CMAKE_CONFIG_TEMPLATE      "cmake/config.cmake.in")
set(VNEPOGODIN_NN_CMAKE_CONFIG_DIR           "${CMAKE_CURRENT_BINARY_DIR}")
set(VNEPOGODIN_NN_CMAKE_VERSION_CONFIG_FILE  "${VNEPOGODIN_NN_CMAKE_CONFIG_DIR}/${PROJECT_NAME}ConfigVersion.cmake")
set(VNEPOGODIN_NN_CMAKE_PROJECT_CONFIG_FILE  "${VNEPOGODIN_NN_CMAKE_CONFIG_DIR}/${PROJECT_NAME}Config.cmake")
set(VNEPOGODIN_NN_CMAKE_PROJECT_TARGETS_FILE "${VNEPOGODIN_NN_CMAKE_CONFIG_DIR}/${PROJECT_NAME}Targets.cmake")
set(VNEPOGODIN_NN_PKGCONFIG_INSTALL_DIR      "${CMAKE_INSTALL_LIBDIR}/pkgconfig")


if(MSVC)
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /O2 /arch:AVX2 /fp:precise /LTCG:INCREMENTAL")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /DEBUG /Od")
else()
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -ffast-math -flto")
endif()

if (NN_LANG)
    set(CMAKE_CXX_STANDARD 17)
    set(NN_BUILD_LANG c++)
    if((NOT MSVC) AND ((${CMAKE_SYSTEM_NAME} MATCHES "Linux") OR (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")))
        set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -mavx2")
    endif()

    set(VNEPOGODIN_NN_DEFS SIMDJSON_THREADS_ENABLED)
else()
    set(NN_BUILD_LANG c)
    set(VNEPOGODIN_NN_DEFS)
endif()
set(NN_BUILD_LANG_SOURCES_DIR "${PROJECT_SOURCE_DIR}/src/${NN_BUILD_LANG}")

if (NN_MultipleHeaders)
    set(VNEPOGODIN_NN_INCLUDE_BUILD_DIR "${PROJECT_SOURCE_DIR}/include/${NN_BUILD_LANG}")

    if (NN_LANG)
        set(VNEPOGODIN_NN_HEADERS
            ${VNEPOGODIN_NN_INCLUDE_BUILD_DIR}/vnepogodin/third_party/json/simdjson.hpp

            ${VNEPOGODIN_NN_INCLUDE_BUILD_DIR}/vnepogodin/helper.hpp
            ${VNEPOGODIN_NN_INCLUDE_BUILD_DIR}/vnepogodin/Matrix.hpp
            ${VNEPOGODIN_NN_INCLUDE_BUILD_DIR}/vnepogodin/nn.hpp
        )
        set(VNEPOGODIN_NN_SOURCES
            ${NN_BUILD_LANG_SOURCES_DIR}/third_party/json/simdjson.cpp

            ${NN_BUILD_LANG_SOURCES_DIR}/Matrix.cpp
            ${NN_BUILD_LANG_SOURCES_DIR}/nn.cpp
        )
    else()
        set(VNEPOGODIN_NN_HEADERS
            ${VNEPOGODIN_NN_INCLUDE_BUILD_DIR}/vnepogodin/third_party/json-c/arraylist.h
            ${VNEPOGODIN_NN_INCLUDE_BUILD_DIR}/vnepogodin/third_party/json-c/json_object.h
            ${VNEPOGODIN_NN_INCLUDE_BUILD_DIR}/vnepogodin/third_party/json-c/linkhash.h
            ${VNEPOGODIN_NN_INCLUDE_BUILD_DIR}/vnepogodin/third_party/json-c/printbuf.h
            ${VNEPOGODIN_NN_INCLUDE_BUILD_DIR}/vnepogodin/third_party/json-c/json_tokener.h
            ${VNEPOGODIN_NN_INCLUDE_BUILD_DIR}/vnepogodin/third_party/json-c/json.h

            ${VNEPOGODIN_NN_INCLUDE_BUILD_DIR}/vnepogodin/matrix.h
            ${VNEPOGODIN_NN_INCLUDE_BUILD_DIR}/vnepogodin/nn.h
        )

        set(VNEPOGODIN_NN_SOURCES
            ${NN_BUILD_LANG_SOURCES_DIR}/third_party/json-c/arraylist.c
            ${NN_BUILD_LANG_SOURCES_DIR}/third_party/json-c/json_object.c
            ${NN_BUILD_LANG_SOURCES_DIR}/third_party/json-c/linkhash.c
            ${NN_BUILD_LANG_SOURCES_DIR}/third_party/json-c/printbuf.c
            ${NN_BUILD_LANG_SOURCES_DIR}/third_party/json-c/json_tokener.c

            ${NN_BUILD_LANG_SOURCES_DIR}/matrix.c
            ${NN_BUILD_LANG_SOURCES_DIR}/nn.c
        )
    endif()

    if (NN_LANG)
        set(VNEPOGODIN_NN_PKGCONFIG_NAME "${CMAKE_CURRENT_SOURCE_DIR}/cmake/pkg-config.pc.in")
    else()
        set(VNEPOGODIN_NN_PKGCONFIG_NAME "${CMAKE_CURRENT_SOURCE_DIR}/cmake/pkg-config-c.pc.in")
    endif()

    message(STATUS "Using the multi-header code from ${VNEPOGODIN_NN_INCLUDE_BUILD_DIR}")
else()
    set(VNEPOGODIN_NN_INCLUDE_BUILD_DIR "${PROJECT_SOURCE_DIR}/single_include/${NN_BUILD_LANG}")

    if (NN_LANG)
        set(VNEPOGODIN_NN_PKGCONFIG_NAME "${CMAKE_CURRENT_SOURCE_DIR}/cmake/pkg-config-single_include.pc.in")
    else()
        set(VNEPOGODIN_NN_PKGCONFIG_NAME "${CMAKE_CURRENT_SOURCE_DIR}/cmake/pkg-config-c-single_include.pc.in")
    endif()
    message(STATUS "Using the single-header code from ${VNEPOGODIN_NN_INCLUDE_BUILD_DIR}")
endif()

##
## TARGET
## create target and add include path
##
if (NOT NN_MultipleHeaders)
    add_library(${VNEPOGODIN_NN_TARGET_NAME} INTERFACE)
    target_include_directories(
        ${VNEPOGODIN_NN_TARGET_NAME}
        INTERFACE
        $<BUILD_INTERFACE:${VNEPOGODIN_NN_INCLUDE_BUILD_DIR}>
        $<INSTALL_INTERFACE:include>
    )
    if (NN_LANG)
        target_compile_features(${VNEPOGODIN_NN_TARGET_NAME} INTERFACE cxx_std_17)

        ## add debug view definition file for msvc (natvis)
        if (MSVC)
            set(VNEPOGODIN_ADD_NATVIS TRUE)
            set(VNEPOGODIN_NATVIS_FILE "vnepogodin_nn.natvis")
            target_sources(
                ${VNEPOGODIN_NN_TARGET_NAME}
                INTERFACE
                $<INSTALL_INTERFACE:${VNEPOGODIN_NATVIS_FILE}>
                $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/${VNEPOGODIN_NATVIS_FILE}>
            )
        endif()
    else()
        target_compile_features(${VNEPOGODIN_NN_TARGET_NAME} INTERFACE c_std_99)
    endif()

else()
    add_library(${VNEPOGODIN_NN_TARGET_NAME}
        ${VNEPOGODIN_NN_SOURCES}
        ${VNEPOGODIN_NN_HEADERS}
    )
    set_target_properties(${VNEPOGODIN_NN_TARGET_NAME} PROPERTIES
        VERSION 0.5.0
        SOVERSION 0
    )

    list(APPEND CMAKE_TARGETS ${VNEPOGODIN_NN_TARGET_NAME})
    target_include_directories(${VNEPOGODIN_NN_TARGET_NAME}
        PUBLIC
        $<BUILD_INTERFACE:${VNEPOGODIN_NN_INCLUDE_BUILD_DIR}>
        $<BUILD_INTERFACE:${PROJECT_BINARY_DIR}>
        $<INSTALL_INTERFACE:include>
    )

    # Allow to build static and shared libraries at the same time
    if (BUILD_STATIC_LIBS AND BUILD_SHARED_LIBS)
        set(VNEPOGODIN_NN_STATIC_LIB ${PROJECT_NAME}-static)
        add_library(${VNEPOGODIN_NN_STATIC_LIB} STATIC
            ${VNEPOGODIN_NN_SOURCES}
            ${VNEPOGODIN_NN_HEADERS}
        )

        # rename the static library
        if (NOT MSVC)
            set_target_properties(${VNEPOGODIN_NN_STATIC_LIB} PROPERTIES
                OUTPUT_NAME ${VNEPOGODIN_NN_TARGET_NAME}
            )
        endif()
        list(APPEND CMAKE_TARGETS ${VNEPOGODIN_NN_STATIC_LIB})
    endif()

    if (UNIX OR MINGW OR CYGWIN)
        list(APPEND CMAKE_REQUIRED_DEFINITIONS -D_GNU_SOURCE)
    endif()

    if (NN_LANG)
        target_compile_features(${VNEPOGODIN_NN_TARGET_NAME} PUBLIC cxx_std_17)

        ## add debug view definition file for msvc (natvis)
        if (MSVC)
            set(VNEPOGODIN_ADD_NATVIS TRUE)
            set(VNEPOGODIN_NATVIS_FILE "vnepogodin_nn.natvis")
            target_sources(
                ${VNEPOGODIN_NN_TARGET_NAME}
                PUBLIC
                $<INSTALL_INTERFACE:${VNEPOGODIN_NATVIS_FILE}>
                $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/${VNEPOGODIN_NATVIS_FILE}>
            )
        endif()
    else()
        target_compile_features(${VNEPOGODIN_NN_TARGET_NAME} PUBLIC c_std_99)
    endif()
endif()

add_library(project_warnings INTERFACE)

include(${NN_BUILD_LANG_SOURCES_DIR}/cmake/CompilerWarnings.cmake)
set_project_warnings(project_warnings)

add_library(${PROJECT_NAME}::${VNEPOGODIN_NN_TARGET_NAME} ALIAS ${VNEPOGODIN_NN_TARGET_NAME})

add_compile_definitions(${VNEPOGODIN_NN_DEFS} ${CMAKE_THREAD_DEFS_INIT})
add_compile_options(${CMAKE_CXX_FLAGS} ${CMAKE_THREAD_DEFS_INIT})

if (NOT NN_MultipleHeaders)
    target_link_libraries(${PROJECT_NAME} INTERFACE ${CMAKE_THREAD_LIBS_INIT})
else()
    target_link_libraries(${PROJECT_NAME} PUBLIC ${CMAKE_THREAD_LIBS_INIT})
endif()

# Install a pkg-config file, so other tools can find this.
CONFIGURE_FILE(
  ${VNEPOGODIN_NN_PKGCONFIG_NAME}
  "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}.pc"
)

##
## TESTS
## create and configure the unit test target
##
if (NN_BuildTests)
    include(CTest)
    enable_testing()
    add_subdirectory(src/${NN_BUILD_LANG}/tests)
endif()

##
## INSTALL
## install header files, generate and install cmake config files for find_package()
##
include(CMakePackageConfigHelpers)
# use a custom package version config file instead of
# write_basic_package_version_file to ensure that it's architecture-independent
# https://github.com/nlohmann/json/issues/1697
configure_file(
    "cmake/${PROJECT_NAME}ConfigVersion.cmake.in"
    ${VNEPOGODIN_NN_CMAKE_VERSION_CONFIG_FILE}
    @ONLY
)
configure_file(
    ${VNEPOGODIN_NN_CMAKE_CONFIG_TEMPLATE}
    ${VNEPOGODIN_NN_CMAKE_PROJECT_CONFIG_FILE}
    @ONLY
)

if (NN_Install)
    install(
        DIRECTORY ${VNEPOGODIN_NN_INCLUDE_BUILD_DIR}/vnepogodin
        DESTINATION ${VNEPOGODIN_NN_INCLUDE_INSTALL_DIR}
    )
    install(
        FILES ${VNEPOGODIN_NN_CMAKE_PROJECT_CONFIG_FILE} ${VNEPOGODIN_NN_CMAKE_VERSION_CONFIG_FILE}
        DESTINATION ${VNEPOGODIN_NN_CONFIG_INSTALL_DIR}
    )

    if (VNEPOGODIN_ADD_NATVIS)
        install(
            FILES ${VNEPOGODIN_NATVIS_FILE}
            DESTINATION .
        )
    endif()
    export(
        TARGETS ${VNEPOGODIN_NN_TARGET_NAME}
        NAMESPACE ${PROJECT_NAME}::
        FILE ${VNEPOGODIN_NN_CMAKE_PROJECT_TARGETS_FILE}
    )
    install(
        TARGETS ${VNEPOGODIN_NN_TARGET_NAME}
        EXPORT ${VNEPOGODIN_NN_TARGETS_EXPORT_NAME}
        INCLUDES DESTINATION ${VNEPOGODIN_NN_INCLUDE_INSTALL_DIR}
    )
    install(
        EXPORT ${VNEPOGODIN_NN_TARGETS_EXPORT_NAME}
        NAMESPACE ${PROJECT_NAME}::
        DESTINATION ${VNEPOGODIN_NN_CONFIG_INSTALL_DIR}
    )
    install(
        FILES "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}.pc"
        DESTINATION ${VNEPOGODIN_NN_PKGCONFIG_INSTALL_DIR}
    )

    # uninstall
    add_custom_target(uninstall
        COMMAND cat ${PROJECT_BINARY_DIR}/install_manifest.txt | xargs rm
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    )
endif()
