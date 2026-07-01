include(FetchContent)
include(GoogleTest)

if(POLICY CMP0135)
    cmake_policy(SET CMP0135 NEW)
endif()

find_package(GTest QUIET)

if(NOT GTest_FOUND)
    set(BUILD_GMOCK OFF CACHE BOOL "" FORCE)
    set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

    FetchContent_Declare(
        googletest
        URL https://github.com/google/googletest/archive/refs/tags/release-1.12.1.zip
    )

    FetchContent_MakeAvailable(googletest)
endif()

if(TARGET GTest::gtest_main)
    set(SHARED_PTR_GTEST_MAIN_TARGET GTest::gtest_main)
elseif(TARGET GTest::Main)
    set(SHARED_PTR_GTEST_MAIN_TARGET GTest::Main)
elseif(TARGET gtest_main)
    set(SHARED_PTR_GTEST_MAIN_TARGET gtest_main)
else()
    message(FATAL_ERROR "GoogleTest main target was not found")
endif()

add_executable(shared_ptr_tests
    shared_ptr_construction_tests.cpp
    shared_ptr_observer_tests.cpp
    shared_ptr_copy_tests.cpp
    shared_ptr_assignment_tests.cpp
    shared_ptr_reset_swap_tests.cpp
    shared_ptr_lifetime_tests.cpp
    weak_ptr_construction_tests.cpp
    weak_ptr_observer_lock_tests.cpp
    weak_ptr_assignment_reset_swap_tests.cpp
    shared_weak_interaction_tests.cpp
)

target_link_libraries(shared_ptr_tests
    PRIVATE
        shared_ptr
        ${SHARED_PTR_GTEST_MAIN_TARGET}
)

gtest_discover_tests(shared_ptr_tests)
