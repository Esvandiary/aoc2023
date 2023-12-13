cmake_minimum_required(VERSION 3.21)
cmake_policy(VERSION 3.21)

set(CMAKE_C_STANDARD 11)

project(Day12 C)

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

add_executable(Day12 "main.c")
target_link_libraries(Day12 PRIVATE Threads::Threads)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -static-libgcc -static -march=native -mtune=native")
set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -O3")
set_target_properties(Day12 PROPERTIES LINK_SEARCH_START_STATIC ON)
set_target_properties(Day12 PROPERTIES LINK_SEARCH_END_STATIC ON)
target_link_options(Day12 PRIVATE "-static-libgcc")
