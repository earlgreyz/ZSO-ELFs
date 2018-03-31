cmake_minimum_required(VERSION 3.0)
project(ZSO_ELFs C)

set(CMAKE_C_FLAGS "-std=c11")
set(CMAKE_C_STANDARD 11)

find_package(Curses REQUIRED)
include_directories(${CURSES_INCLUDE_DIR})

add_executable(ZSO_ELFs main.c alienos.c emulator.c emulator.h random.c random.h graphics.h graphics.c)
target_link_libraries(ZSO_ELFs ${CURSES_LIBRARIES})