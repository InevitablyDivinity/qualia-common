cmake_minimum_required(VERSION 3.21)
project(QualiaCommon)

option(USE_ASAN OFF)
option(USE_UBSAN OFF)
option(USE_TSAN OFF)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Ensure MSVC is using C++20
if(MSVC)
  add_compile_options(/std:c++latest)
endif()

# Unix expects pthreads
if(UNIX)
  set(THREADS_PREFER_PTHREAD_FLAG ON)
endif()
find_package(Threads REQUIRED)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
  if(USE_ASAN)
    message(":: Address Sanitisers enabled")

    if(MSVC)
      add_compile_options(/fsanitize=address)
      add_link_options(/fsanitize=address)
    else()
      add_compile_options(-fno-omit-frame-pointer -fsanitize=address)
      add_link_options(-fno-omit-frame-pointer -fsanitize=address)
    endif()
  endif()

  if(USE_UBSAN)
    message(":: Undefined Behaviour Sanitisers enabled")
    add_compile_options(-fsanitize=undefined)
    add_link_options(-fsanitize=undefined)
  endif()

  if(USE_TSAN)
    message(":: Thread Sanitisers enabled")
    add_compile_options(-fsanitize=thread)
    add_link_options(-fsanitize=thread)
  endif()
endif()

add_library(QlCommon INTERFACE)

target_link_libraries(
  QlCommon
  INTERFACE
    Threads::Threads
)

target_include_directories(
  QlCommon
  INTERFACE
    "include"
)

add_subdirectory(thirdparty/googletest)
add_subdirectory(tests)
