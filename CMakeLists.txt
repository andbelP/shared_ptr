cmake_minimum_required(VERSION 3.10)

project(proj)

add_library(shared_ptr INTERFACE)
target_include_directories(shared_ptr INTERFACE ${CMAKE_SOURCE_DIR})

add_executable(prog main.cpp)

target_link_libraries(prog PRIVATE shared_ptr)