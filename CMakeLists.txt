cmake_minimum_required(VERSION 3.23)

project(SharedPtr LANGUAGES CXX)

add_library(shared_ptr INTERFACE)
target_sources(shared_ptr
    INTERFACE
    FILE_SET HEADERS
    BASE_DIRS include
    FILES 
        include/shared_ptr/weak_ptr.hpp
        include/shared_ptr/shared_ptr.hpp
        include/shared_ptr/control_block.hpp
)

