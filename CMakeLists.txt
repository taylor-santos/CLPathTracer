cmake_minimum_required(VERSION 3.15)
project(CLPathTracer C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_EXTENSIONS OFF)
SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -pedantic -Wextra -Wformat -Wno-unused-parameter")

find_package(OpenCL REQUIRED)
find_package(glfw3 3.3 REQUIRED)
find_package(gl3w REQUIRED)
include_directories(${OpenCL_INCLUDE_DIR} ${glfw_INCLUDE_DIR} ${gl3w_INCLUDE_DIR} include)

file(GLOB SRC "src/*.c")

add_executable(CLPathTracer ${SRC})
target_link_libraries(CLPathTracer glfw gl3w ${OpenCL_LIBRARY})