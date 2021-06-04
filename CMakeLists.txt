cmake_minimum_required(VERSION 3.13)
project(projekt1 C)

set(CMAKE_C_STANDARD 99)

find_package(Threads)

add_library(logger logger.c logger.h)
target_link_libraries(logger ${CMAKE_THREAD_LIBS_INIT} m)
add_executable(mykill mykill.c)
add_executable(main main.c)
target_link_libraries(main logger)
