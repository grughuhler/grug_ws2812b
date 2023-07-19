add_executable(grug_ws2812b)

# enable usb output, disable uart output
pico_enable_stdio_usb(grug_ws2812b 1)
pico_enable_stdio_uart(grug_ws2812b 0)


pico_generate_pio_header(grug_ws2812b ${CMAKE_CURRENT_LIST_DIR}/grug_ws2812b.pio)

target_sources(grug_ws2812b PRIVATE grug_ws2812b.c)

target_link_libraries(grug_ws2812b PRIVATE
        pico_stdlib
        hardware_pio
	hardware_adc)

pico_add_extra_outputs(grug_ws2812b)

# add url via pico_set_program_url
example_auto_set_url(grug_ws2812b)
