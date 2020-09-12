cmake_minimum_required(VERSION 3.1)

project(lc2kicad)

set(EXEC lc2kicad)

set(CMAKE_CXX_STANDARD 14)

IF (MINGW OR CYGWIN)
	add_link_options(-static)
ENDIF ()

IF (CMAKE_BUILD_TYPE EQUAL "DEBUG" AND MSVC)
    add_link_options(/ZI) # Edit and continue
ENDIF()

include_directories(inc)

file(GLOB SOURCES src/*.cpp icon/*.rc)

add_executable(${EXEC} ${SOURCES})

install(TARGETS lc2kicad DESTINATION ${CMAKE_INSTALL_PREFIX})

