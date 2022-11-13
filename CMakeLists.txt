cmake_minimum_required(VERSION 3.21)
project(QualiaCommon)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Ensure MSVC is using C++20
if(MSVC)
  add_compile_options(/Zstd:c++latest)
endif()

# Unix expects pthreads
if(UNIX)
  set(THREADS_PREFER_PTHREAD_FLAG ON)
endif()
find_package(Threads REQUIRED)

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
