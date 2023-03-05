cmake_minimum_required(VERSION 3.21)
project(mariobox)

set(CMAKE_CXX_STANDARD 20)

include_directories(src)

add_executable(mariobox src/main.cpp
        src/olcPixelGameEngine.h src/olcPixelGameEngine.cpp

        src/core/SystemBus.cpp src/core/SystemBus.h
        src/core/C6502.cpp src/core/C6502.h
        src/core/Memory.cpp src/core/Memory.h
        src/core/Cartridge.cpp src/core/Cartridge.h
        src/core/Mapper.cpp src/core/Mapper.h
        src/core/NES.cpp src/core/NES.h

        src/core/ppu/NESColor.h
        src/core/ppu/P2C02.cpp src/core/ppu/P2C02.h
        src/core/ppu/Palettes.cpp src/core/ppu/Palettes.h

        src/platform/Canvas.h

        src/debug/ASM.cpp src/debug/ASM.h
        src/Logger.cpp src/Logger.h src/platform/olc/olcCanvas.cpp src/platform/olc/olcCanvas.h)

find_package(OpenGL REQUIRED)
include_directories(${OPENGL_INCLUDE_DIRS})
target_link_libraries(mariobox ${OPENGL_LIBRARIES})

find_package(GLUT REQUIRED)
include_directories(${GLUT_INCLUDE_DIRS})
target_link_libraries(mariobox ${GLUT_LIBRARY})