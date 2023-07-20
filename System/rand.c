#include "stm32f10x.h" // Device header
#include "rand.h"
#include "stdlib.h"

uint8_t rand_get(uint8_t bound)
{
    return rand()%bound;
}
