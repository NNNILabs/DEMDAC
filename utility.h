#ifndef UTILITY_H
#define UTILITY_H

#define LED_PIN 25

float pioDivider = 100;
int pin = 0;

char get_on_bits(unsigned char byte);
short random_bits(unsigned char no, unsigned long seed);

void pioSetup();

#endif