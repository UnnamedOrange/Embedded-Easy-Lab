cmake_minimum_required(VERSION 3.5)

# Toolchain.
set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_PROCESSOR arm)

set(CMAKE_C_FLAGS_INIT -pedantic)
set(CMAKE_CXX_FLAGS_INIT -pedantic)

project(
    lab-socket-client
    VERSION 0.0.0
    LANGUAGES C
)

add_executable(${PROJECT_NAME})
target_sources(${PROJECT_NAME} PRIVATE main.c)
