/* ============================================================
 * 01 - CHIP ALIVE  (PIC16F877A + 20 MHz crystal)
 * ------------------------------------------------------------
 * Before ANY peripheral, prove three things at once:
 *   - power reaches VDD/VSS,
 *   - the 20 MHz crystal is oscillating,
 *   - the PICkit actually programmed the chip.
 * A 1 Hz heartbeat LED on RB1 (pin 34) confirms all three.
 *
 * Hardware around the chip:
 *   MCLR (1)        -> +5V through a 10k resistor
 *   VDD (11, 32)    -> +5V        VSS (12, 31) -> GND
 *   OSC1/OSC2 (13/14)-> 20 MHz crystal, each leg to GND via ~22pF
 *   100nF across VDD/VSS close to the chip + 470uF bulk on the rail
 *   RB1 (34)        -> LED + 220 ohm -> GND
 *
 * NOTE: LVP is OFF below - that frees RB3 and keeps ICSP clean.
 * In Proteus the VDD/VSS pins are hidden; don't wire them in sim.
 * ============================================================ */

#include <xc.h>

#pragma config FOSC = HS        // 20 MHz crystal
#pragma config WDTE = OFF       // watchdog off
#pragma config PWRTE = OFF
#pragma config BOREN = ON
#pragma config LVP = OFF        // low-voltage prog OFF (frees RB3, clean ICSP)
#pragma config CPD = OFF
#pragma config WRT = OFF
#pragma config CP = OFF

#define _XTAL_FREQ 20000000

void main(void) {
    TRISB1 = 0;                 // RB1 = output (heartbeat)
    while (1) {
        RB1 = 1; __delay_ms(500);
        RB1 = 0; __delay_ms(500);
    }
}
