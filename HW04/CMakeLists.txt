cmake_minimum_required(VERSION 3.7)
project(HW4)

set(CMAKE_C_STANDARD 99)

set(SOURCE_FILES hw4.c)
add_executable(HW4 ${SOURCE_FILES})
target_link_libraries(HW4 pthread)