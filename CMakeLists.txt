cmake_minimum_required(VERSION 3.21)

if (${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_BINARY_DIR})
    message(FATAL_ERROR "In-source builds are not recommended. Consider re-running CMake with `-B build` instead.")
endif ()

project(Array)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD_LIBRARIES "")
set(CMAKE_INTERPROCEDURAL_OPTIMIZATION_RELEASE ON)
set(CMAKE_VS_JUST_MY_CODE_DEBUGGING ON)

include(FetchContent)

message("Fetching dependency: doctest")
FetchContent_Declare(doctest
        GIT_REPOSITORY https://github.com/doctest/doctest.git
        GIT_TAG v2.4.11
)
FetchContent_MakeAvailable(doctest)

if (MSVC)
    add_compile_options(/W4)
else ()
    add_compile_options(-Wall -Wextra -Wpedantic -Wshadow -Wconversion -Wsign-conversion)
endif ()

file(GLOB_RECURSE TESTS_SRC CONFIGURE_DEPENDS "${CMAKE_SOURCE_DIR}/tests/*")

add_executable(Tests ${TESTS_SRC})

target_link_libraries(Tests PRIVATE doctest::doctest)

target_include_directories(Tests PRIVATE ${CMAKE_SOURCE_DIR}/include)
