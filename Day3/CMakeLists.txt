cmake_minimum_required(VERSION 3.21)
cmake_policy(VERSION 3.21)

set(CMAKE_C_STANDARD 11)

project(Day3 C)

add_executable(Day3 "main.c")

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -static-libgcc -static -O3 -march=native -mtune=native")
set_target_properties(Day3 PROPERTIES LINK_SEARCH_START_STATIC ON)
set_target_properties(Day3 PROPERTIES LINK_SEARCH_END_STATIC ON)
target_link_options(Day3 PRIVATE "-static-libgcc")
