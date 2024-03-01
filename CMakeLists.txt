cmake_minimum_required (VERSION 2.8)
project(hw3)

set(CMAKE_C_FLAGS "-std=c11 -Wall -Wextra -Wshadow -Werror -D_XOPEN_SOURCE=500")
set(CMAKE_CXX_FLAGS "-std=c++11 -Wall -Wextra -Wshadow -Werror -Wno-sign-compare -D_XOPEN_SOURCE=500")

#GoogleTest requires at least C++14
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories("${PROJECT_SOURCE_DIR}/include")

# build a dynamic library called libblock_store.so
# note that the prefix lib will be automatically added in the filename.


# Add our include directory to CMake's search paths.
include_directories(include)

add_library(bitmap src/bitmap.c)
add_library(block_store src/block_store.c)

# make an executable
add_executable(${PROJECT_NAME}_test test/tests.cpp)
target_compile_definitions(${PROJECT_NAME}_test PRIVATE)
target_link_libraries(${PROJECT_NAME}_test gtest pthread block_store)
