#include "utility.h"
#include "pico/stdlib.h"
#include "hardware/pio.h"

void pioSetup()
{
    PIO pio = pio0;
    uint sm = pio_claim_unused_sm(pio, true);
    uint offset = pio_add_program(pio, &output_program);
    float div = pioDivider;
    output_program_init(pio, sm, offset, pin, div);
    pio_sm_set_enabled(pio, sm, true);
}

//Skaytacium territory: skaytacium.com/random
char get_on_bits(unsigned char byte) {
    unsigned char i = 8, on = 0;
    while (i--) {
        if ((byte >> i) & 1) {
            on++;
        }
    }
    return on;
}

short random_bits(unsigned char no, unsigned long seed) {
    unsigned char ret = 0;

    for (char i = 0; i < 32; i++) {
        if ((seed >> i) & 1) {
            if (get_on_bits(ret) != no) {
                ret |= 128;
            } else {
                continue;
            }
            ret = (ret << 1) | (ret >> 7);
        } else {
            ret = (ret << 1) | (ret >> 7);
        }
    }
    return ret;
}
