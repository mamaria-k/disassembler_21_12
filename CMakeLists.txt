cmake_minimum_required(VERSION 3.20)
project(lab_03)

set(CMAKE_CXX_STANDARD 17)
include_directories(include)

add_executable(lab_03 src/main.cpp src/elf_parser.cpp include/elf_parser.h src/disassembler.cpp include/disassembler.h)
