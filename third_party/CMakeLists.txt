set(BUILD_SHARED_LIBS OFF CACHE INTERNAL "")

# configure Vc
set(Vc_ENABLE_TESTING OFF CACHE INTERNAL "" FORCE)

# header directories
add_subdirectory(Vc)
