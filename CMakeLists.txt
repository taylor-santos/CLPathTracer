cmake_minimum_required(VERSION 3.15)
project(CLPathTracer C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_EXTENSIONS OFF)
SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -pedantic -Wextra -Wformat -Wno-unused-parameter -Wsequence-point")

find_package(OpenCL REQUIRED)
find_package(glfw3 3.3 REQUIRED)
find_package(gl3w REQUIRED)
find_package(BISON 3.5.0 REQUIRED)
find_package(FLEX 2.6.4 REQUIRED)

bison_target(
        OBJParser ${CMAKE_SOURCE_DIR}/src/obj_parser.y
        ${CMAKE_CURRENT_BINARY_DIR}/obj_parser.c
)
flex_target(
        OBJScanner ${CMAKE_SOURCE_DIR}/src/obj_scanner.l
        ${CMAKE_CURRENT_BINARY_DIR}/obj_scanner.c
        DEFINES_FILE ${CMAKE_CURRENT_BINARY_DIR}/obj_scanner.h
)
add_flex_bison_dependency(OBJScanner OBJParser)

include_directories(
        ${OpenCL_INCLUDE_DIR}
        ${glfw_INCLUDE_DIR}
        ${gl3w_INCLUDE_DIR}
        ${CMAKE_CURRENT_BINARY_DIR}
        include)

file(GLOB SRC "src/*.c")

add_executable(CLPathTracer
        ${BISON_OBJParser_OUTPUTS}
        ${FLEX_OBJScanner_OUTPUTS}
        ${SRC})
target_link_libraries(CLPathTracer
        glfw
        gl3w
        ${OpenCL_LIBRARY})