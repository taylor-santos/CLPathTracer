cmake_minimum_required(VERSION 3.15)
project(OpenCLProject C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_EXTENSIONS OFF)
SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -pedantic -Wextra -Wformat -Wno-unused-parameter")

find_package(OpenCL REQUIRED)
include_directories(${OpenCL_INCLUDE_DIR} include)

file(GLOB SRC "src/*.c")

add_executable(OpenCLProject ${SRC})
target_link_libraries(OpenCLProject ${OpenCL_LIBRARY})