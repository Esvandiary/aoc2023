cmake_minimum_required(VERSION 3.25)
cmake_policy(VERSION 3.25)

set(CMAKE_C_STANDARD 11)

project(Day1 C)

add_executable(Day1 "main.c")

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -static-libgcc -static -Os")
set_target_properties(Day1 PROPERTIES LINK_SEARCH_START_STATIC ON)
set_target_properties(Day1 PROPERTIES LINK_SEARCH_END_STATIC ON)
target_link_options(Day1 PRIVATE "-static-libgcc")
