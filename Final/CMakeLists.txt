cmake_minimum_required(VERSION 3.7)
project(Final)

set(CMAKE_C_STANDARD 99)

set(SOURCE_FILES server.c sharedArea.h SVDecomposition.c SVDecomposition.h QRFact.h QRFact.c pInverse.c pInverse.h)
set(SOURCE_FILES2 clients.c sharedArea.h)
add_executable(server ${SOURCE_FILES})
add_executable(clients ${SOURCE_FILES2})
target_link_libraries(server pthread)
target_link_libraries(clients pthread)