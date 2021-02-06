cmake_minimum_required(VERSION 3.15.0 FATAL_ERROR)

set(CMAKE_SYSTEM_VERSION 10.0.17134.0 CACHE STRING "" FORCE)

project(Seaurchin CXX)
option(USE_CLANG OFF)

################################################################################
# Sub-projects
################################################################################
add_subdirectory(libsus)
add_subdirectory(Seaurchin)
add_subdirectory(SeaurchinStandardLibrary)
