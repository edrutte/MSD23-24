include_directories(${CMAKE_SOURCE_DIR}/include)
include_directories(${CMAKE_SOURCE_DIR}/chess-library/include)
include_directories(${CMAKE_SOURCE_DIR}/lcd1602_i2c)
add_executable(main
		${CMAKE_SOURCE_DIR}/chess-library/src
		${CMAKE_SOURCE_DIR}/lcd1602_i2c/lcd1602_i2c.c
		Adafruit_MFRC630.cpp
		Adafruit_MFRC630_consts.c
		chess.cpp
		main.c
		pi_i2c.c
		pi_rfid.cpp
		pi_spi.c
		stepper.c
)
add_dependencies(main Stockfish)
#Inspired by https://github.com/axr/solar-cmake/blob/master/TargetArch.cmake
set(armdetect_c_code "
#ifdef __aarch64__
#error isarm
#endif
")
set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)
try_compile(NOTARM SOURCE_FROM_VAR isarm.c armdetect_c_code)
if(NOT ${NOTARM})
target_link_libraries(main wiringPi)
endif()
