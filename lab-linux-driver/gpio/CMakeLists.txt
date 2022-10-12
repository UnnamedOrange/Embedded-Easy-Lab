cmake_minimum_required(VERSION 3.5)

project(
    lab-linux-driver-gpio
    VERSION 0.0.0
    LANGUAGES C
)

set(KERNEL_DIRECTORY "/home/orange/linux-custom" CACHE PATH "Kernel directory")
set(CROSS_COMPILE "/opt/aarch64-buildroot-linux-gnu_sdk-buildroot/bin/aarch64-none-linux-gnu-" CACHE STRING "Cross compile prefix")

set(SOURCE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/main.c")

string(REPLACE ".c" ".o" OUTPUT_PATH "${SOURCE_PATH}")
get_filename_component(OUTPUT_PATH "${OUTPUT_PATH}" NAME)
add_custom_target(${PROJECT_NAME} ALL
    COMMAND ${CMAKE_COMMAND} -E copy "${SOURCE_PATH}" "${CMAKE_CURRENT_BINARY_DIR}/"
    DEPENDS "${SOURCE_PATH}"
)
add_custom_command(TARGET ${PROJECT_NAME}
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E echo "obj-m := ${OUTPUT_PATH}" > ${CMAKE_CURRENT_BINARY_DIR}/Makefile
    COMMAND make ARCH=arm64 CROSS_COMPILE=${CROSS_COMPILE} -C ${KERNEL_DIRECTORY} M=${CMAKE_CURRENT_BINARY_DIR} modules
)
