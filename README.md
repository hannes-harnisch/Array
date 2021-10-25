# FixedList

A single-header implementation of a compile-time fixed-capacity, dynamic array that is allocated automatically, so without any internal heap allocation, with a standard-library-like interface. It provides a more lightweight alternative to ``std::vector`` and variable-length arrays, since most implementations of ``std::vector`` will perform an expensive dynamic allocation even for small amounts of elements, and VLAs are not universally supported and only completely safe when it is known that the length does not exceed a set size, in which case a constant-length array would also be faster. Also, C-style arrays don't allow non-default-constructible types, while ``FixedList`` does. ``FixedList`` aims to address these use cases, while also providing the following:

* Conformance with C++20 and later.
* Implementing the interface of ``std::vector`` except for methods related to its reallocation features.
* Strong exception safety guarantees where performance wouldn't otherwise need to be sacrificed. Documentation comments indicate which methods are exception-safe and under which circumstances. If the comment above a non-``const`` method does not mention exception safety, then that method is not exception-safe and the container is in an invalid state should an exception be thrown.
* Absolutely minimal memory footprint. This is achieved by only storing the array itself as well as its element count, with a dependent type. The type of the count member depends on the alignment requirements of the element type and the specified capacity. It is chosen such that it never increases the alignment requirements of the ``FixedList`` beyond the alignment requirements of the element type, as long as it can represent all possible element counts given the specified capacity.
* Absolutely minimal ABI footprint, such that if the element type is, for example, trivially destructible, then the FixedList is also trivially destructible. Trivial copy constructibility, copy assignability, move constructibility and move assignability are adopted from the element type as well. This is achieved through C++20 ``requires`` clauses on these methods.
* Asserts in debug builds when indexing out of bounds, pushing into a full list, popping from an empty list and similar situations.
* Alternatively it provides safe versions of all modifying methods, prefixed with ``try_``, whose return value indicates whether the method succeeded. Documentation comments provide more details for each method.
* Allows using custom assertions.
* No spillage of implementation details.
* Allowing straightforward modification of class and method names as well as the namespace to fit your codebase's naming convention, if need be.

## To do

* Add converting constructors for FixedLists where the source value type is conversion-constructible to the target value type.
* Improve internal use of ``std::memmove``.
* Refactor modifying member functions to take const iterators.