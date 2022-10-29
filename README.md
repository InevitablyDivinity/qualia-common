# qualia-common
A personal replacement library for C++'s Standard Template Library.

## Usage
```
add_subdirectory("qualia-common")

add_executable(example
  "main.cpp"
)

target_link_libraries(
  example
  PRIVATE
    "QlCommon"
)
```
