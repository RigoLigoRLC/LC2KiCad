cmake_minimum_required(VERSION 3.1)

project(lc2kicad)

set(EXEC lc2kicad)

set(CMAKE_CXX_STANDARD 14)

IF (MINGW OR CYGWIN)

set(CMAKE_EXE_LINKER_FLAGS -static)

ENDIF ()

include_directories(inc)

file(GLOB SOURCES src/*.cpp icon/*.rc)

add_executable(${EXEC} ${SOURCES})

install(TARGETS lc2kicad DESTINATION ${CMAKE_INSTALL_PREFIX})

