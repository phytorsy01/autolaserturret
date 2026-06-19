/* ============================================================
 * SCANNING LASER TURRET - final  (PIC16F877A @ 20 MHz)
 * ------------------------------------------------------------
 * The servo sweeps 0-180. When the HC-SR04 sees something within
 * DETECT_CM, the servo stops and holds that angle while the laser
 * and buzzer fire; when the target clears, it resumes sweeping.
 *
 * A push button toggles the whole system between RUN and STOP:
 *   RUN  -> GREEN LED on, turret active
 *   STOP -> RED LED on, servo frozen, laser + buzzer off
 *
 * Servo PWM is generated in software and timed by Timer1 (no
 * interrupts), so a button-press or sensor read never disturbs it.
 *
 * Wiring:
 *   PORTB (turret):
 *     Servo signal -> RB0 (33)   own 5V supply, GND bridged to PIC GND
 *     Laser  S -> RB1 (34)       Buzzer + -> RB2 (35)
 *     HC-SR04 Trig -> RB3 (36)   Echo -> RB4 (37)   VCC->5V  GND->GND
 *   PORTD (user interface):
 *     Green LED -> RD0 (19) via 220R -> GND
 *     Red   LED -> RD1 (20) via 220R -> GND
 *     Button    -> RD2 (21): button to GND, 10k pull-up from RD2 to 5V
 *                  (so RD2 = HIGH normally, LOW when pressed)
 *   PORTA (sensitivity knob):
 *     Pot wiper -> RA0 / AN0 (2);  pot outer legs -> +5V and GND
 *     Turns the trigger distance between 5 and 50 cm, live.
 * ============================================================ */

#include <xc.h>

#pragma config FOSC = HS, WDTE = OFF, PWRTE = ON, BOREN = OFF
#pragma config LVP = OFF, CPD = OFF, WRT = OFF, CP = OFF

#define _XTAL_FREQ 20000000

/* ---- pins ---- */
#define SERVO     RB0
#define LASER     RB1
#define BUZZER    RB2
#define TRIG      RB3
#define ECHO      RB4
#define GREEN_LED RD0
#define RED_LED   RD1
#define BUTTON    RD2          /* active low */

/* ---- settings ---- */
#define DETECT_MIN_CM  5       /* pot fully one way -> trigger at 5 cm  */
#define DETECT_MAX_CM 50       /* pot fully other way -> trigger at 50 cm */
#define MAX_RANGE_CM  60       /* furthest distance we bother measuring */
#define CLOSE_US    1000       /* ~0 deg   */
#define OPEN_US     2000       /* ~180 deg */
#define CENTER_US   ((CLOSE_US + OPEN_US) / 2)   /* 90 deg = home */
#define SWEEP_STEP     2
#define CLEAR_FRAMES   3

unsigned int servo_us = CENTER_US;

/* one precise servo pulse, timed by Timer1 (0.8 us/tick) */
void servo_pulse(unsigned int us) {
    unsigned int target = (us * 5) / 4;
    SERVO = 1;
    TMR1H = 0; TMR1L = 0; TMR1ON = 1;
    while ((((unsigned int)TMR1H << 8) | TMR1L) < target) ;
    TMR1ON = 0;
    SERVO = 0;
}

/* distance in cm; 0 = nothing within MAX_RANGE_CM. Bails early so it
   never stalls the loop on far / no-object readings. */
unsigned int read_cm(void) {
    unsigned int g, ticks;
    unsigned int limit = MAX_RANGE_CM * 72;

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

/* read the sensitivity pot on RA0 -> 0..1023 (right-justified 10-bit) */
unsigned int read_adc(void) {
    __delay_us(20);                 /* let the input settle (acquisition) */
    GO_nDONE = 1;                   /* start conversion */
    while (GO_nDONE) ;              /* wait until it finishes */
    return ((unsigned int)ADRESH << 8) | ADRESL;
}

void main(void) {
    ADCON1 = 0xCE;              /* RA0 analog, rest digital, right-justified, Fosc/64 */
    ADCON0 = 0x81;              /* ADC on, channel AN0 */
    TRISA0 = 1;                 /* RA0 = analog input (sensitivity pot) */
    /* PORTB: servo/laser/buzzer/trig out, echo in */
    TRISB0 = 0; TRISB1 = 0; TRISB2 = 0; TRISB3 = 0; TRISB4 = 1;
    /* PORTD: LEDs out, button in */
    TRISD0 = 0; TRISD1 = 0; TRISD2 = 1;

    LASER = 0; BUZZER = 0; SERVO = 0;
    GREEN_LED = 0; RED_LED = 0;
    T1CON = 0x20;               /* Timer1: 1:4 prescaler, internal clock, off */

    int angle = 90, dir = SWEEP_STEP;
    unsigned char detected = 0, clear_count = 0;
    unsigned char running = 1, btn_prev = 0;

    for (unsigned char k = 0; k < 40; k++) { servo_pulse(CENTER_US); __delay_ms(18); }

    while (1) {
        /* ---- button: toggle RUN/STOP on each press ---- */
        unsigned char btn_now = (BUTTON == 0);          /* pressed = low */
        if (btn_now && !btn_prev) { running = !running; __delay_ms(30); }
        btn_prev = btn_now;

        if (!running) {
            /* STOPPED: hold still, outputs off, red light */
            RED_LED = 1; GREEN_LED = 0;
            LASER = 0; BUZZER = 0;
            servo_pulse(servo_us);      /* keep holding position steady */
            __delay_ms(16);
            continue;
        }

        /* RUNNING */
        RED_LED = 0; GREEN_LED = 1;
        servo_pulse(servo_us);

        unsigned int detect_cm = DETECT_MIN_CM +
            (unsigned int)(((unsigned long)read_adc() * (DETECT_MAX_CM - DETECT_MIN_CM)) / 1023);
        unsigned int cm = read_cm();
        if (cm > 0 && cm <= detect_cm) { detected = 1; clear_count = 0; }
        else if (detected) { if (++clear_count >= CLEAR_FRAMES) detected = 0; }

        if (detected) {
            LASER = 1; BUZZER = 1;          /* target: hold + fire */
            __delay_ms(16);
        } else {
            LASER = 0; BUZZER = 0;          /* clear: sweep */
            angle += dir;
            if (angle >= 180)    { angle = 180; dir = -SWEEP_STEP; }
            else if (angle <= 0) { angle = 0;   dir =  SWEEP_STEP; }
            servo_us = CLOSE_US + (unsigned int)((long)(OPEN_US - CLOSE_US) * angle / 180);
            __delay_ms(16);
        }
    }
}
