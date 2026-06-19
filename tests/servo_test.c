/* ============================================================
 * TEST: SERVO  (SG90 on RB0, PIC16F877A @ 20 MHz)
 * Sweeps 0-180-0 using Timer1-timed software PWM. Run alone to
 * confirm the servo, its power, and common ground.
 *   Signal -> RB0 (33)   VCC -> own 5V rail   GND -> bridged to PIC GND
 * ============================================================ */
#include <xc.h>
#pragma config FOSC = HS, WDTE = OFF, PWRTE = ON, BOREN = OFF
#pragma config LVP = OFF, CPD = OFF, WRT = OFF, CP = OFF
#define _XTAL_FREQ 20000000
#define SERVO RB0

void servo_pulse(unsigned int us) {
    unsigned int target = (us * 5) / 4;       /* 0.8 us/tick */
    SERVO = 1;
    TMR1H = 0; TMR1L = 0; TMR1ON = 1;
    while ((((unsigned int)TMR1H << 8) | TMR1L) < target) ;
    TMR1ON = 0; SERVO = 0;
}

void main(void) {
    TRISB0 = 0; SERVO = 0;
    T1CON = 0x20;                              /* Timer1 1:4 prescaler */
    while (1) {
        for (unsigned int a = 0; a <= 180; a += 2) {
            servo_pulse(1000 + (unsigned int)((long)1000 * a / 180));
            __delay_ms(18);
        }
        for (unsigned int a = 180; a > 0; a -= 2) {
            servo_pulse(1000 + (unsigned int)((long)1000 * a / 180));
            __delay_ms(18);
        }
    }
}
