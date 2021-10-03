cmake_minimum_required(VERSION 3.16)
project(pmbench)

set(CMAKE_CXX_STANDARD 17)

find_package(benchmark REQUIRED)

add_executable(${PROJECT_NAME} main.cpp)
target_link_libraries(${PROJECT_NAME} PRIVATE pmemobj)
target_link_libraries(${PROJECT_NAME} PRIVATE benchmark::benchmark)