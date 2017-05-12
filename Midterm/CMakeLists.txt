cmake_minimum_required(VERSION 3.6)
project(Midterm)

set(CMAKE_C_STANDARD 99 )

set(SOURCE_FILES timeServer.c )
set(SOURCE_FILES2 seeWhat.c)
set(SOURCE_FILES3 showResults.c)
add_executable(timeServer ${SOURCE_FILES} )
add_executable(seeWhat ${SOURCE_FILES2})
add_executable(showResults ${SOURCE_FILES3})
target_link_libraries(timeServer m -D_POSIX_C_SOURCE)
target_link_libraries(seeWhat m -D_POSIX_C_SOURCE)
target_link_libraries(showResults m -D_POSIX_C_SOURCE)
