cmake_minimum_required(VERSION 3.5)

project(
    lab-socket-server
    VERSION 0.0.0
    LANGUAGES C
)

add_executable(${PROJECT_NAME})
target_sources(${PROJECT_NAME} PRIVATE main.c)
