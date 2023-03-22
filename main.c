#include "hardware/pio.h"
#include "pico/stdlib.h"
#include "pico/rand.h"

#include "output.pio.h"

float pioDivider = 100.0;
int startPin = 0;

char get_on_bits(unsigned char byte);
short random_bits(unsigned char no, unsigned long seed);

void main()
{
    PIO pio = pio0;
    uint sm = pio_claim_unused_sm(pio, true);
    uint offset = pio_add_program(pio, &output_program);
    output_program_init(pio, sm, offset, startPin, pioDivider);

    while (1) 
    {
        pio_sm_put_blocking(pio, sm, random_bits(5, get_rand_32()));
    }
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