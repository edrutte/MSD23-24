include_directories(${CMAKE_SOURCE_DIR}/include)
add_executable(main ../MFRC630/mfrc630.c main.c pi_rfid.c pi_spi.c pi_i2c.c stepper.c)
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