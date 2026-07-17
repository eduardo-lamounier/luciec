cmake_minimum_required(VERSION 3.15)
project(luciec C)

set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

enable_testing()

find_path(CRITERION_INCLUDE_DIR NAMES criterion/criterion.h)
find_library(CRITERION_LIBRARY NAMES criterion)

if(NOT CRITERION_INCLUDE_DIR OR NOT CRITERION_LIBRARY)
  message(FATAL_ERROR "Criterion is not installed in your system.")
endif()

# Main application

add_executable( luciec
  src/luciec.c
  src/lexing.c
  src/logging.c
  src/utils/string_view.c
)

target_include_directories( luciec PRIVATE
  include/
)

target_compile_options( luciec PRIVATE
  -Wall
  -Wextra
  -g
)

# Testing

add_executable( tests
  src/utils/string_view.c
  src/logging.c
  src/lexing.c
  tests/testing.c
)

target_include_directories( tests PRIVATE 
  include/
  ${CRITERION_INCLUDE_DIR}
)

target_compile_options( tests PRIVATE 
  -Wall
  -Wextra
  -g
)

target_link_libraries( tests PRIVATE
  ${CRITERION_LIBRARY}
)

add_test(NAME AllUnitTests COMMAND tests)

