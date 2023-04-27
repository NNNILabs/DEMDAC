#include <stdio.h>

#include "pico/stdlib.h"
#include "pico/stdlib.h"
#include "pico/rand.h"
#include "pico/multicore.h"
#include "hardware/dma.h"
#include "hardware/pio.h"
#include "hardware/clocks.h"

#include "output.pio.h"

#define newline printf("\n")

uint32_t xorshift32(uint32_t *state) {
    uint32_t x = *state;
    x ^= x << 13;
    x ^= x >> 17;
    x ^= x << 5;
    *state = x;
    return x;
}

uint8_t random_bits(uint8_t num_bits, uint32_t seed) {
    uint8_t result = 0;
    uint8_t remaining_bits = 8;

    for (uint8_t i = 0; i < num_bits; i++) {
        uint32_t rand_val = xorshift32(&seed);
        uint8_t bit_pos = (rand_val % remaining_bits);
        uint8_t bit = 0;
        uint8_t pos = 0;

        for (uint8_t j = 0; j <= bit_pos; j++) {
            while ((result >> pos) & 1) {
                pos++;
            }
            if (j < bit_pos) {
                pos++;
            }
        }

        bit = 1 << pos;
        result |= bit;
        remaining_bits--;
    }

    return result;
}

void core2()
{
    //Life indicator
    gpio_init(25);
    gpio_set_dir(25, true);

    while(true)
    {
        gpio_put(25, true);
        sleep_ms(500);
        gpio_put(25, false);
        sleep_ms(500);
    }
}

int main()
{
    stdio_init_all();

    set_sys_clock_khz(100000, true);

    gpio_init(25);
    gpio_set_dir(25, true);

    uint OUT_PIN_NUMBER = 0;
    uint NPINS = 8;
    uint bufdepth = 1024;

    uint8_t awg_buff[bufdepth] __attribute__((aligned(256)));

    for (int i = 0; i < bufdepth; i = i + 1)
    {
        awg_buff[i] = random_bits(5, get_rand_32());
    }

    PIO pio = pio0;
    uint sm = pio_claim_unused_sm(pio, true);
    uint offset = pio_add_program(pio, &output_program);
    output_program_init(pio, sm, offset, OUT_PIN_NUMBER);

    int wave_dma_chan_a = dma_claim_unused_channel(true);
    int wave_dma_chan_b = dma_claim_unused_channel(true);

    dma_channel_config wave_dma_chan_a_config = dma_channel_get_default_config(wave_dma_chan_a);
    channel_config_set_chain_to(&wave_dma_chan_a_config, wave_dma_chan_b);
    channel_config_set_dreq(&wave_dma_chan_a_config, DREQ_PIO0_TX0);
    channel_config_set_ring(&wave_dma_chan_a_config, false, 8);

    dma_channel_config wave_dma_chan_b_config = dma_channel_get_default_config(wave_dma_chan_b);
    channel_config_set_chain_to(&wave_dma_chan_b_config, wave_dma_chan_a);
    channel_config_set_dreq(&wave_dma_chan_b_config, DREQ_PIO0_TX0);
    channel_config_set_ring(&wave_dma_chan_b_config, false, 8);

    dma_channel_configure(
        wave_dma_chan_a,
        &wave_dma_chan_a_config,
        &pio0_hw->txf[sm],
        awg_buff,
        bufdepth,
        false);

    dma_channel_configure(
        wave_dma_chan_b,
        &wave_dma_chan_b_config,
        &pio0_hw->txf[sm],
        awg_buff,
        bufdepth,
        false);

    dma_start_channel_mask(1u << wave_dma_chan_a);

    multicore_launch_core1(core2);

    char input;

	while(true) 
    {
        input = getchar();
        newline;
        switch(input)
        {
            case '0':
                printf("Code 0/8");
                newline;
                for (int i = 0; i < bufdepth; i = i + 1) 
                {
		            awg_buff[i] = 0; 
	            }
                break;

            case '1':
                printf("Code 1/8");
                newline;
                for (int i = 0; i < bufdepth; i = i + 1) 
                {
		            awg_buff[i] = random_bits(1, get_rand_32()); 
	            }
                break;

            case '2':
                printf("Code 2/8");
                newline;
                for (int i = 0; i < bufdepth; i = i + 1) 
                {
		            awg_buff[i] = random_bits(2, get_rand_32()); 
	            }
                break;

            case '3':
                printf("Code 3/8");
                newline;
                for (int i = 0; i < bufdepth; i = i + 1) 
                {
		            awg_buff[i] = random_bits(3, get_rand_32()); 
	            }
                break;

            case '4':
                printf("Code 4/8");
                newline;
                for (int i = 0; i < bufdepth; i = i + 1) 
                {
		            awg_buff[i] = random_bits(4, get_rand_32()); 
	            }
                break;

            case '5':
                printf("Code 5/8");
                newline;
                for (int i = 0; i < bufdepth; i = i + 1) 
                {
		            awg_buff[i] = random_bits(5, get_rand_32()); 
	            }
                break;

            case '6':
                printf("Code 6/8");
                newline;
                for (int i = 0; i < bufdepth; i = i + 1) 
                {
		            awg_buff[i] = random_bits(6, get_rand_32()); 
	            }
                break;

            case '7':
                printf("Code 7/8");
                newline;
                for (int i = 0; i < bufdepth; i = i + 1) 
                {
		            awg_buff[i] = random_bits(7, get_rand_32()); 
	            }
                break;

            case '8':
                printf("Code 8/8");
                newline;
                for (int i = 0; i < bufdepth; i = i + 1) 
                {
		            awg_buff[i] = 255; 
	            }
                break;
            
            default:
                printf("Unrecognized command!");
                newline;
                break;
        }
    }
}