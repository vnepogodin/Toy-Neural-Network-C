function(set_project_warnings project_name)
    option(WARNINGS_AS_ERROR "Treat compiler warnings as error" FALSE)

    set(MSVC_WARNINGS
        /W4 # Base
        /w14242 # Conversion
        /w14287 # unsigned/negative constant mismatch
        /w14296 # expression is always 'boolean_value'
        /w14311 # pointer trunc from one tipe to another

        /w14555 # expresion has no effect; expected expression with side-effect
        /w14619 # pragma warning
        /w14640 # Enable warning on thread; static member

        /w14826 # Conversion from one tipe to another is sign-extended cause unexpected runtime behavior.
        /X
    )

    set(CLANG_WARNINGS
        -Wall
        -Wextra # standard
        -Wshadow

        -Wcast-align
        -Wunused

        -Wconversion
        -Wsign-conversion

        -Wformat=2
    )

    if(WARNINGS_AS_ERRORS)
        set(CLANG_WARNINGS ${CLANG_WARNINGS} -Werror)
        set(MSVC_WARNINGS ${MSVC_WARNINGS} /WX)
    endif()

    set(GCC_WARNINGS
        ${CLANG_WARNINGS}
        -Wmisleading-indentation

        -Wduplicated-cond
        -Wduplicated-branches
        -Wlogical-op

        -Wuseless-cast
    )

    if(MSVC)
        set(PROJECT_WARNINGS ${MSVC_WARNINGS})
    elseif(CMAKE_C_COMPILER_ID STREQUAL "Clang")
        set(PROJECT_WARNINGS ${CLANG_WARNINGS})
    else()
        set(PROJECT_WARNINGS ${GCC_WARNINGS})
    endif()

    target_compile_options(${project_name} INTERFACE ${PROJECT_WARNINGS})
endfunction()
