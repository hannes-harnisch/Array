# Array

A single-header implementation of a fixed-size heap-allocated array with a standard-library-like interface. It provides a more lightweight alternative to ``std::vector`` when dynamic resizing is not needed since ``std::vector`` has to store overhead _because_ it is dynamically resizable. Array aims to achieve the following:

* Conformance with C++17 and later.
* Being a drop-in replacement for a ``const std::vector``, ``std::unique_ptr<T[]>`` and any use of ``new[]``.
* A familiar interface similar to standard containers and of course full iterator support along with some neat extras.
* Custom allocator support like all standard containers.
* The same high level of memory safety as standard containers as well as strong exception safety guarantees.
* Asserts when accessing the array out of bounds in debug builds. Allows using custom assertions.
* Absolutely no spillage of implementation details.
* Allowing easy modification of class and method names as well as the namespace to fit your codebase's naming convention.
* Minimal memory overhead, achieved through two ways:
  * Having only 2 word-sized members as compared to ``std::vector``'s 3 word-sized members (in typical standard library implementations).
  * Using ``std::allocator``, which typically just uses ``new``. This elides some overhead associated with ``new[]``, which has to keep track of the allocated size at the start of the allocated block in most implementations.

# Interface

Coming soon.
