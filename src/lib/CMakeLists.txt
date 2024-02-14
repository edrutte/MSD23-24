include_directories(${CMAKE_SOURCE_DIR}/include)
add_executable(main main.c pi_gpio.c pi_spi.c)
target_link_libraries(main gpiod)
