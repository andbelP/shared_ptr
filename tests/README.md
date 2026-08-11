# SharedPtr tests

Ожидается, что основной проект содержит CMake-цель `shared_ptr`, которая
передаёт каталог с этими заголовками через `target_include_directories`.

Подключение тестов в корневом `CMakeLists.txt`:

```cmake
include(CTest)
add_subdirectory(tests)
```

Сборка и запуск:

```sh
cmake -S . -B build
cmake --build build -j
ctest --test-dir build --output-on-failure
```

Для сборки с AddressSanitizer и UndefinedBehaviorSanitizer:

```sh
cmake -S . -B build -DENABLE_SANITIZERS=ON
```

GoogleTest 1.15.2 автоматически загружается через `FetchContent`.
