cmake_minimum_required(VERSION 3.14)

project(proj LANGUAGES CXX)

add_library(shared_ptr INTERFACE)
target_include_directories(shared_ptr INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})

target_compile_features(shared_ptr INTERFACE cxx_std_11)

enable_testing()
add_subdirectory(tests)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)