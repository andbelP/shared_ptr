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
        include/shared_ptr/shared_ptr_constructors.hpp
        include/shared_ptr/shared_ptr_operators.hpp
        include/shared_ptr/shared_ptr_modifiers.hpp
        include/shared_ptr/shared_ptr_observers.hpp
)

option(BUILD_TESTING "Build tests for shared_ptr" ON)

if(BUILD_TESTING)
    enable_testing()
    add_subdirectory(tests)
endif()
