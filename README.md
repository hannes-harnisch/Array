# Array

A collection of array-based containers offering various tradeoffs.

## `VarArray`

A single-header implementation of a fixed-size heap-allocated array with a standard-library-like interface.

It provides a more lightweight alternative to `std::vector` when dynamic resizing is not needed since `std::vector` has to store overhead _because_ it is dynamically resizable.

`VarArray` aims to achieve the following:

* conformance with C++17 and later
* being a drop-in replacement for a `const std::vector`, `std::unique_ptr<T[]>` and any use of `new[]`
* a familiar interface similar to standard containers and of course full iterator support along with some neat extras
* custom allocator support like all standard containers
* the same high level of memory safety as standard containers as well as strong exception safety guarantees.
* asserts in debug builds when indexing out of bounds
* allows using custom assertions
* absolutely no spillage of implementation details
* allowing easy modification of class and method names as well as the namespace to fit your codebase's naming convention
* minimal memory overhead, achieved through two ways:
  * having only 2 word-sized members as compared to `std::vector`'s 3 word-sized members (in typical standard library implementations)
  * using `std::allocator`, which typically just uses `new`, eliding some overhead associated with `new[]`, which has to keep track of the allocated size at the start of the allocated block in most implementations

### To do
* Providing an alternate allocator for types that correctly overload `new[]`/`delete[]` or `new`/`delete`.

## FixedList

A single-header implementation of a compile-time fixed-capacity, dynamic array that is allocated automatically, so without any internal heap allocation, with a standard-library-like interface.

It provides a more lightweight alternative to `std::vector` and variable-length arrays, since most implementations of `std::vector` will perform an expensive dynamic allocation even for small amounts of elements, and VLAs are not universally supported and only completely safe when it is known that the length does not exceed a set size, in which case a constant-length array would also be faster. Also, C-style arrays don't allow non-default-constructible types, while `FixedList` does.

`FixedList` aims to address these use cases, while also providing the following:

* conformance with C++20 and later
* implementing the interface of `std::vector` except for methods related to its reallocation features
* strong exception safety guarantees where performance wouldn't otherwise need to be sacrificed
  * documentation comments indicate which methods are exception-safe and under which circumstances
  * if the comment above a non-`const` method does not mention exception safety, then that method is not exception-safe and the container is in an invalid state should an exception be thrown
* absolutely minimal memory footprint
  * achieved by only storing the array itself as well as its element count
  * the count member's type depends on the alignment requirements of the element type and the specified capacity 
  * chosen such that it never increases the alignment requirements of the `FixedList` beyond the alignment requirements of the element type, as long as it can represent all possible element counts given the specified capacity
* absolutely minimal ABI footprint, such that if the element type is, for example, trivially destructible, then the `FixedList` is also trivially destructible; trivial copy constructibility, copy assignability, move constructibility and move assignability are derived similarly
* asserts in debug builds when indexing out of bounds, pushing into a full list, popping from an empty list and similar situations
* alternatively it provides safe versions of all modifying methods, prefixed with `try_`, whose return value indicates whether the method succeeded
* allows using custom assertions
* no spillage of implementation details
* allowing straightforward modification of class and method names as well as the namespace to fit your codebase's naming convention, if need be

### To do

* add converting constructors for FixedLists where the source value type is conversion-constructible to the target value type
* improve internal use of `std::memmove`
* refactor modifying member functions to take const iterators
