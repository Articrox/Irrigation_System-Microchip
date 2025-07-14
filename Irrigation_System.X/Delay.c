#include "definitions.h"                // SYS function prototypesementation
// Note: These assume a 48MHz CPU clock - adjust if different
void delay_us(uint32_t us) {
    // For a 48MHz clock, one cycle is approximately 20.83ns
    // So we need roughly 48 cycles per microsecond
    uint32_t cycles = us * 48;
    
    // Use built-in NOP instruction to prevent optimization
    for(volatile uint32_t i = 0; i < cycles; i++) {
        __asm__ volatile ("nop");
    }
}

void delay_ms(uint32_t ms) {
    for(uint32_t i = 0; i < ms; i++) {
        delay_us(1000);
    }
}