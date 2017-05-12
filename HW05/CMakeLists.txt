cmake_minimum_required(VERSION 3.7)
project(HW05)

set(CMAKE_C_STANDARD 99)

set(SOURCE_FILES main.c)
add_executable(grepThwShandMQ ${SOURCE_FILES})
target_link_libraries(grepThwShandMQ pthread)