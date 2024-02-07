include_directories(${CMAKE_SOURCE_DIR}/include)
add_executable(main main.c pi5_gpio.c)
target_link_libraries(main gpiod)
