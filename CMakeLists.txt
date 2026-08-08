cmake_minimum_required(VERSION 3.15)
project(luciec C)

set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

option(TESTING "Build tests with Criterion (must be installed in your system)" OFF)

if(WIN32)
  set(PLATFORM_SOURCES src/platform_win.c)
elseif(UNIX)
  set(PLATFORM_SOURCES src/platform_unix.c)
else()
  message(FATAL_ERROR "Your platform is not supported.")
endif()

# Main application

add_executable( luciec
  src/luciec.c
  src/types.c
  src/logging.c
  src/lexing.c
  src/parsing.c
  ${PLATFORM_SOURCES}
)

target_include_directories( luciec PRIVATE
  include/
)

# For gcc and clang only:
if(CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
  target_compile_options(luciec PRIVATE -Wall -Wextra -g)
endif()

# Testing

if(TESTING)
  enable_testing()

  find_path(CRITERION_INCLUDE_DIR NAMES criterion/criterion.h)
  find_library(CRITERION_LIBRARY NAMES criterion)

  if(NOT CRITERION_INCLUDE_DIR OR NOT CRITERION_LIBRARY)
    message(FATAL_ERROR "Criterion is not installed in your system.")
  endif()

  add_executable( tests
    src/logging.c
    src/lexing.c
    tests/testing.c
    ${PLATFORM_SOURCES}
  )

  target_include_directories( tests PRIVATE 
    include/
    ${CRITERION_INCLUDE_DIR}
  )

  # Also for gcc and clang only:
  if(CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options( tests PRIVATE -Wall -Wextra -g)
  endif()

  target_link_libraries( tests PRIVATE
    ${CRITERION_LIBRARY}
  )

  add_test(NAME AllUnitTests COMMAND tests)
endif()

