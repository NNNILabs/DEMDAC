#ifndef UTILITY_H
#define UTILITY_H

float pioDivider = 100.0;
int startPin = 0;

char get_on_bits(unsigned char byte);
short random_bits(unsigned char no, unsigned long seed);

void pioSetup(void);

#endif