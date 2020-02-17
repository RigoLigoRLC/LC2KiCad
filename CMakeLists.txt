project(lc2kicad)

set(EXEC lc2kicad)

set(CMAKE_CXX_STANDARD 14)

include_directories(inc)

file(GLOB SOURCES src/*.cpp)

add_executable(${EXEC} ${SOURCES})
