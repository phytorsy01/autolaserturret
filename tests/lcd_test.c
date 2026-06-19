/* ============================================================
 * LCD TEST - datasheet init  (16x2 ST7066/HD44780, 4-bit on PORTD)
 * PIC16F877A @ 20 MHz.  Run this ALONE to verify the LCD.
 *
 * Wiring (datasheet pin numbers):
 *   1 VSS->GND   2 VDD->5V   3 V0->contrast pot   5 RW->GND
 *   4 RS->RD2    6 E->RD3
 *   11 DB4->RD4  12 DB5->RD5  13 DB6->RD6  14 DB7->RD7
 *   (pins 7-10 = DB0-DB3 stay UNCONNECTED in 4-bit mode)
 *
 * Expect exactly:   line1  PIC16F877A LCD
 *                   line2  Test 0123456789
 * If you see that, the LCD and wiring are correct.
 * If it is still garbled, the data lines (11-14 -> RD4-RD7) are
 * swapped or on the wrong pins - that is the only thing left.
 * ============================================================ */

#include <xc.h>

#pragma config FOSC = HS, WDTE = OFF, PWRTE = ON, BOREN = OFF
#pragma config LVP = OFF, CPD = OFF, WRT = OFF, CP = OFF

#define _XTAL_FREQ 20000000

#define LCD_RS RD2
#define LCD_EN RD3

void lcd_pulse(void) { LCD_EN = 1; __delay_us(2); LCD_EN = 0; __delay_us(50); }

void lcd_nib(unsigned char n) {
    RD4 = (n >> 0) & 1; RD5 = (n >> 1) & 1;
    RD6 = (n >> 2) & 1; RD7 = (n >> 3) & 1;
    lcd_pulse();
}

void lcd_cmd(unsigned char c) {
    LCD_RS = 0; lcd_nib(c >> 4); lcd_nib(c & 0x0F);
    if (c == 0x01 || c == 0x02) __delay_ms(2);   // clear / home are slow
    else __delay_us(50);
}

void lcd_chr(unsigned char d) {
    LCD_RS = 1; lcd_nib(d >> 4); lcd_nib(d & 0x0F);
    __delay_us(50);
}

void lcd_str(const char *s) { while (*s) lcd_chr(*s++); }

/* datasheet 4-bit power-on sequence (page 16) */
void lcd_init(void) {
    TRISD = 0x03;                 // RD2..RD7 outputs
    LCD_RS = 0; LCD_EN = 0;
    __delay_ms(50);               // wait > 40 ms after power-up
    lcd_nib(0x03); __delay_ms(5);
    lcd_nib(0x03); __delay_us(150);
    lcd_nib(0x03); __delay_us(150);
    lcd_nib(0x02); __delay_us(150);   // switch to 4-bit
    lcd_cmd(0x28);                // function set: 4-bit, 2 lines, 5x8
    lcd_cmd(0x08);                // display OFF
    lcd_cmd(0x01);                // clear display
    lcd_cmd(0x06);                // entry mode, auto-increment
    lcd_cmd(0x0C);                // display ON, cursor off
}

void main(void) {
    lcd_init();
    lcd_cmd(0x80); lcd_str("PIC16F877A LCD");
    lcd_cmd(0xC0); lcd_str("Test 0123456789");
    while (1) { }
}
