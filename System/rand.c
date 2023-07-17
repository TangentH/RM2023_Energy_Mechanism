#include "stm32f10x.h" // Device header
#include "rand.h"
#include "time.h"
#include "stdlib.h"

extern uint32_t systime_ms;    //used as random seed

uint8_t rand_get(uint8_t bound)
{
    srand(systime_ms);
    return rand()%bound;
}
