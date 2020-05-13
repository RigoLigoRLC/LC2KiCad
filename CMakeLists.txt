project(lc2kicad)

set(EXEC lc2kicad)

set(CMAKE_CXX_STANDARD 14)

IF(NOT APPLE)

set(CMAKE_EXE_LINKER_FLAGS -static)

ENDIF (APPLE)

include_directories(inc)

file(GLOB SOURCES src/*.cpp icon/*.rc)

add_executable(${EXEC} ${SOURCES})