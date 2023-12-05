<<<<<<< HEAD
set(CMAKE_CXX_STANDARD 17)
cmake_minimum_required(VERSION 3.16)
set(CMAKE_CXX_FLAGS "-Wall -O0 -g -ggdb3")
# set(CMAKE_CXX_FLAGS "-Wall -O3")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O0 -g")
set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -O0 -g")
=======
cmake_minimum_required(VERSION 3.16)
>>>>>>> 0fc62b07da8290e400c2858776d6f0d23f21ae8a
project(RMDB)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "-Wall -O0 -g -ggdb3")
# set(CMAKE_CXX_FLAGS "-Wall -O3")

set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O0 -g")
set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -O0 -g")


enable_testing()
add_subdirectory(src)
add_subdirectory(deps)
