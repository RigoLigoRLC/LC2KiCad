project(lc2kicad)

set(EXEC lc2kicad)

set(CMAKE_CXX_STANDARD 14)

set(CMAKE_EXE_LINKER_FLAGS -static)

include_directories(inc)

file(GLOB SOURCES src/*.cpp icon/*.rc)

add_executable(${EXEC} ${SOURCES})

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/CMakeModules/")

include(GetGitRevisionDescription)

get_git_head_revision(GIT_REFSPEC GIT_SHA1)
add_definitions("-DGIT_SHA1=${GIT_SHA1}")