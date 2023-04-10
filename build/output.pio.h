// -------------------------------------------------- //
// This file is autogenerated by pioasm; do not edit! //
// -------------------------------------------------- //

#pragma once

#if !PICO_NO_HARDWARE
#include "hardware/pio.h"
#endif

// ------ //
// output //
// ------ //

#define output_wrap_target 0
#define output_wrap 0

static const uint16_t output_program_instructions[] = {
            //     .wrap_target
    0x6008, //  0: out    pins, 8                    
            //     .wrap
};

#if !PICO_NO_HARDWARE
static const struct pio_program output_program = {
    .instructions = output_program_instructions,
    .length = 1,
    .origin = -1,
};

static inline pio_sm_config output_program_get_default_config(uint offset) {
    pio_sm_config c = pio_get_default_sm_config();
    sm_config_set_wrap(&c, offset + output_wrap_target, offset + output_wrap);
    return c;
}

void output_program_init(PIO pio, uint sm, uint offset, uint pin) 
{
    pio_sm_config c = output_program_get_default_config(offset);
    pio_gpio_init(pio, pin);
    pio_gpio_init(pio, pin + 1);
    pio_gpio_init(pio, pin + 2);
    pio_gpio_init(pio, pin + 3);
    pio_gpio_init(pio, pin + 4);
    pio_gpio_init(pio, pin + 5);
    pio_gpio_init(pio, pin + 6);
    pio_gpio_init(pio, pin + 7);
    pio_sm_set_consecutive_pindirs(pio, sm, pin, 8, true);
    sm_config_set_clkdiv(&c, 100);
    sm_config_set_out_pins(&c, pin, 8);
    sm_config_set_out_shift(&c, false, true, 8);
    pio_sm_init(pio, sm, offset, &c);
    pio_sm_set_enabled(pio, sm, true);
}

#endif
