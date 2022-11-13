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

## Types
Name | Description
--- | ---
`ql::String` | An SSBO-enabled alternative to and wrapper for C strings.
`ql::Vector` | A resizable array.
`ql::List` | A singly-linked list.
`ql::Function` | An SSBO-enabled object encapsulating the functionality of callable types (function pointers, function objects, lambdas). Unlike a function pointer, it is capable of wrapping a lambda with captures.
`ql::UniquePtr` | A smart pointer that automatically deletes the pointed object upon leaving scope.
`ql::SharedPtr` | A smart pointer that shares the pointed object among other shared pointers. Automatically deletes the object when it's released by all shareholders.
`ql::WeakPtr` | A smart pointer that transfers ownership of the pointed object to a `ql::SharedPtr`. The pointed object is deleted if ownership is not taken before leaving scope.
`ql::Tuple` | A standard layout tuple capable of using structured bindings.
`ql::BitFlags` | An object to help ease the use of bit flags.
`ql::Library` | An object encapsulating the functionality of a shared library.
`ql::Thread` | An object encapsulating the functionality of a thread.
`ql::Iterator` | An object that represents the position of an item within a container and can be used to traverse items within said container.
`ql::Variant` | An object capable of holding one of various specified types.
