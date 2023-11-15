cmake_minimum_required(VERSION 3.5.0)

project(AutoChessBoard
LANGUAGES C CXX
)

include(GNUInstallDirs)
include(ExternalProject)

find_program(MAKE_EXECUTABLE NAMES gmake make REQUIRED)

# From https://www.scivision.dev/cmake-external-project-makefile/
set(STOCKFISH_LIBRARY ${CMAKE_INSTALL_FULL_LIBDIR}/${CMAKE_STATIC_LIBRARY_PREFIX}Stockfish${CMAKE_STATIC_LIBRARY_SUFFIX})

ExternalProject_Add(Stockfish
PREFIX ${CMAKE_SOURCE_DIR}/Stockfish
SOURCE_DIR ${CMAKE_SOURCE_DIR}/Stockfish/src
CONFIGURE_COMMAND ${MAKE_EXECUTABLE} -C ${CMAKE_SOURCE_DIR}/Stockfish/src net
UPDATE_DISCONNECTED true
BUILD_COMMAND ${MAKE_EXECUTABLE} -j -C ${CMAKE_SOURCE_DIR}/Stockfish/src profile-build ARCH=armv8
BUILD_BYPRODUCTS ${STOCKFISH_LIBRARY}
BUILD_IN_SOURCE true
)

add_subdirectory(lib)
