cmake_minimum_required(VERSION 3.15)
project(myshell CXX)

set(CMAKE_CXX_STANDARD 17)
SET(CMAKE_CXX_FLAGS "-lreadline")

# Options
option(WARNINGS_AS_ERRORS   "Treat compiler warnings as errors."    OFF)
option(ENABLE_SANITIZERS    "Use sanitizers to detect errors."      OFF)
option(ENABLE_CONAN         "Use Conan as a package manager."       OFF)

# Include fixed CMake configuration
include(cmake/config.cmake)

# Project source compilation
include_directories(inc)
add_library(operations SHARED src/operations.cpp inc/operations/operations.hpp)
add_executable(myshell src/main.cpp)
target_link_libraries(myshell operations)
target_link_libraries(myshell readline)

# Add external packages
find_package(Boost 1.65.1 COMPONENTS program_options filesystem REQUIRED)
include_directories(${Boost_INCLUDE_DIR})
target_link_libraries(myshell Boost::program_options Boost::filesystem)

# Sanitizers configuration
if (ENABLE_SANITIZERS)
    set(SANITIZE_UNDEFINED ON)
    # Only one of Memory, Address, or Thread sanitizers is applicable at the time
    set(SANITIZE_MEMORY ON)
#    set(SANITIZE_ADDRESS ON)
#    set(SANITIZE_THREAD ON)
    find_package(Sanitizers)

    # CMake targets to which apply sanitizers
    add_sanitizers(operations myshell)
endif ()
