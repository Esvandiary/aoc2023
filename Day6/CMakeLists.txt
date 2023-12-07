cmake_minimum_required(VERSION 3.21)
cmake_policy(VERSION 3.21)

set(CMAKE_C_STANDARD 11)

project(Day6 C)

add_executable(Day6 "main.c")
target_link_libraries(Day6 PRIVATE "m")

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -static-libgcc -static -O3 -ffast-math -march=native -mtune=native")
set_target_properties(Day6 PROPERTIES LINK_SEARCH_START_STATIC ON)
set_target_properties(Day6 PROPERTIES LINK_SEARCH_END_STATIC ON)
target_link_options(Day6 PRIVATE "-static-libgcc")
