cmake_minimum_required(VERSION 3.21)
project(nessy)

set(CMAKE_CXX_STANDARD 14)

include_directories(.)
add_executable(nessy main.cpp
        hardware/bus/Bus.cpp hardware/bus/Bus.h
        utils.h
        hardware/C6502.cpp hardware/C6502.h)
