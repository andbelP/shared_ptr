cmake_minimum_required(VERSION 3.20)

project(shared_ptr_tests LANGUAGES CXX)

option(ENABLE_SANITIZERS "Enable AddressSanitizer and UndefinedBehaviorSanitizer" ON)

include(FetchContent)

FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/refs/tags/v1.15.2.zip
  DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)
FetchContent_MakeAvailable(googletest)

enable_testing()
find_package(Threads REQUIRED)

add_executable(shared_ptr_tests
 advanced_types_test.cpp
  aliasing_and_inheritance_test.cpp
#  allocator_test.cpp
  arrays_test.cpp
  assignment_test.cpp
  casts_test.cpp
  comparisons_and_ownership_test.cpp
  constructors_test.cpp
  custom_deleter_test.cpp
  lifetime_test.cpp
  modifiers_test.cpp
  observers_test.cpp
#  thread_safety_test.cpp
  weak_interop_test.cpp
)

target_compile_features(shared_ptr_tests PRIVATE cxx_std_20)
target_link_libraries(shared_ptr_tests PRIVATE
  shared_ptr
  GTest::gtest_main
  Threads::Threads
)

if(ENABLE_SANITIZERS)
  target_compile_options(shared_ptr_tests PRIVATE
    -fsanitize=address,undefined
    -fno-omit-frame-pointer
  )
  target_link_options(shared_ptr_tests PRIVATE
    -fsanitize=address,undefined
    -fno-omit-frame-pointer
  )
endif()

include(GoogleTest)
gtest_discover_tests(shared_ptr_tests)
