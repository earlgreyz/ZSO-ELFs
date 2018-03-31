cmake_minimum_required(VERSION 3.0)
project(ZSO_ELFs C)

set(CMAKE_C_FLAGS "-std=c11")
set(CMAKE_C_STANDARD 11)

find_package(Curses REQUIRED)
include_directories(${CURSES_INCLUDE_DIR})

add_executable(emu main.c alienos.c emulator.c random.c random.h graphics.c)
target_link_libraries(emu ${CURSES_LIBRARIES})