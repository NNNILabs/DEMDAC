#include <stdio.h>
#include <stdlib.h>

#include "pico/stdlib.h"
#include "pico/rand.h"
#include "pico/multicore.h"
#include "hardware/dma.h"
#include "hardware/pio.h"
#include "hardware/clocks.h"
#include "tusb.h"

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

uint8_t random_bits(uint8_t num_bits, uint32_t *state){
    uint32_t seed = xorshift32(state);
    random_bits(num_bits, seed);
    return random_bits(num_bits, seed);
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
    
    // Wait until USB CDC is ready
    while(!tud_cdc_connected()){
        tight_loop_contents();
    }

    multicore_launch_core1(core2);

    int requested_code = -1;
    int last_code = -1;
    char input_buff[32] = {0};
    int new_input = 0;

    // initialize the random number generator
    uint32_t rng_state = get_rand_32();
    xorshift32(&rng_state);

    sleep_ms(100);

	while(true) 
    {
        printf("Enter a number between 0 and 7: ");
        // block until we get a string from the user
        while(!tud_cdc_available()){
            sleep_ms(10);
        }
        // read the string
        new_input = scanf("%s", &input_buff);
        // convert the string to an int
        requested_code = atoi(input_buff);
        // clear the input buffer
        memset(input_buff, 0, sizeof(input_buff));
        // print the number
        printf("%d\n", requested_code);

        if(requested_code < 0 || requested_code > 8){
            printf("Number out of range\n");
            requested_code = 0;
            last_code = 0;
            continue;
        }else if(last_code != requested_code || new_input > 0){
            last_code = requested_code;
            printf("Code %d/8\n", requested_code);
            newline;
            for (int i = 0; i < bufdepth; i = i + 1) 
            {
                awg_buff[i] = random_bits(requested_code, &rng_state); 
            }
        }

    }
}