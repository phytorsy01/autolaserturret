/* ============================================================
 * TEST: HC-SR04  (Trig RB3, Echo RB4, PIC16F877A @ 20 MHz)
 * Lights the laser (RB1) when something is within DETECT_CM.
 * Run alone to confirm the sensor and wiring.
 *   Trig -> RB3 (36)   Echo -> RB4 (37)   VCC -> 5V   GND -> GND
 *   ticks=0 / never lights -> check COMMON GROUND first, then Trig/Echo.
 * ============================================================ */
#include <xc.h>
#pragma config FOSC = HS, WDTE = OFF, PWRTE = ON, BOREN = OFF
#pragma config LVP = OFF, CPD = OFF, WRT = OFF, CP = OFF
#define _XTAL_FREQ 20000000
#define TRIG RB3
#define ECHO RB4
#define LASER RB1
#define DETECT_CM 20

unsigned int read_cm(void) {
    unsigned int g, ticks;
    unsigned int limit = (DETECT_CM + 5) * 72;
    TRIG = 0; __delay_us(2);
    TRIG = 1; __delay_us(10); TRIG = 0;
    g = 0; while (ECHO == 0) { if (++g > 8000) return 0; }
    TMR1H = 0; TMR1L = 0; TMR1ON = 1;
    while (ECHO == 1) {
        ticks = ((unsigned int)TMR1H << 8) | TMR1L;
        if (ticks > limit) { TMR1ON = 0; return 0; }
    }
    TMR1ON = 0;
    return (((unsigned int)TMR1H << 8) | TMR1L) / 72;
}

void main(void) {
    TRISB3 = 0; TRISB4 = 1; TRISB1 = 0;
    LASER = 0;
    T1CON = 0x20;
    while (1) {
        unsigned int cm = read_cm();
        LASER = (cm > 0 && cm <= DETECT_CM) ? 1 : 0;
        __delay_ms(60);
    }
}
