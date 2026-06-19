/* ============================================================
 * TEST: ADC pot  (RA0/AN0, PIC16F877A @ 20 MHz)
 * Blinks the green LED on RD0 - turning the pot changes the blink
 * speed. Proves the ADC reads and tracks the full range.
 *   Pot wiper -> RA0 (2);  pot ends -> +5V and GND
 *   Green LED -> RD0 (19) via 220R -> GND
 * ============================================================ */
#include <xc.h>
#pragma config FOSC = HS, WDTE = OFF, PWRTE = ON, BOREN = OFF
#pragma config LVP = OFF, CPD = OFF, WRT = OFF, CP = OFF
#define _XTAL_FREQ 20000000
#define GREEN RD0

unsigned int read_adc(void) {
    __delay_us(20);
    GO_nDONE = 1;
    while (GO_nDONE) ;
    return ((unsigned int)ADRESH << 8) | ADRESL;   /* 0..1023 */
}

void main(void) {
    ADCON1 = 0xCE;              /* RA0 analog, rest digital, right-justified, Fosc/64 */
    ADCON0 = 0x81;              /* ADC on, AN0 */
    TRISA0 = 1;                 /* RA0 input */
    TRISD0 = 0;                 /* green LED out */

    while (1) {
        unsigned int ms = 40 + read_adc() / 4;     /* ~40..295 ms */
        GREEN ^= 1;
        for (unsigned int i = 0; i < ms; i++) __delay_ms(1);
    }
}
