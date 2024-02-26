cmake_minimum_required(VERSION 3.5.0)

project(Chess
LANGUAGES C CXX
)

include(GNUInstallDirs)
include(ExternalProject)

find_program(MAKE_EXECUTABLE NAMES gmake make REQUIRED)

# From https://www.scivision.dev/cmake-external-project-makefile/

ExternalProject_Add(Stockfish
PREFIX ${CMAKE_SOURCE_DIR}/Stockfish
SOURCE_DIR ${CMAKE_SOURCE_DIR}/Stockfish/src
STAMP_DIR ${CMAKE_BINARY_DIR}/Stockfish-stamp
TMP_DIR ${CMAKE_BINARY_DIR}/Stockfish-tmp
CONFIGURE_COMMAND ${MAKE_EXECUTABLE} -C ${CMAKE_SOURCE_DIR}/Stockfish/src net
BUILD_COMMAND ${MAKE_EXECUTABLE} -j -C ${CMAKE_SOURCE_DIR}/Stockfish/src profile-build
BUILD_IN_SOURCE true
BUILD_ALWAYS false
INSTALL_COMMAND ""
UPDATE_COMMAND ""
)

add_subdirectory(lib)
