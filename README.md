# Scanning Laser Turret — PIC16F877A

A servo sweeps an ultrasonic sensor across 0–180°. When something comes within the
trigger distance, the turret stops, holds that angle, and fires a laser and buzzer until
the target clears, then resumes sweeping. A push button toggles the whole system between
RUN and STOP, two status LEDs report which state it's in, and a potentiometer sets the
trigger distance live.

Built in **MPLAB XC8** for the **PIC16F877A** at a **20 MHz** crystal.

## Features

- **Sweep & detect** — software-PWM servo sweep with HC-SR04 ranging each frame.
- **Lock & fire** — laser + buzzer fire while a target is held; target hysteresis stops it twitching on a single noisy reading.
- **Start/Stop button** — polled each frame, toggles RUN ↔ STOP; in STOP the servo holds steady and outputs go off.
- **Status LEDs** — green = running, yellow = stopped.
- **Sensitivity knob** — a potentiometer on the ADC sets the trigger distance (5–50 cm) live.

## On-chip peripherals used

- **GPIO** — all digital I/O: servo, laser, buzzer, Trig, Echo, the two LEDs, and the button.
- **Timer1** — times the HC-SR04 echo pulse and the width of each servo pulse.
- **ADC** — reads the sensitivity pot on AN0 and maps it to the trigger distance.

No interrupts, no USART, no hardware (CCP) PWM, no Timer0 — the servo PWM is generated in
software and the whole program is one polled loop.

## Pin map

| Function | Pin | Port |
|---|---|---|
| Servo signal | RB0 (33) | PORTB |
| Laser (via 2N2222) | RB1 (34) | PORTB |
| Buzzer (via 2N2222) | RB2 (35) | PORTB |
| HC-SR04 Trig | RB3 (36) | PORTB |
| HC-SR04 Echo | RB4 (37) | PORTB |
| Green LED (running) | RD0 (19) | PORTD |
| Yellow LED (stopped) | RD1 (20) | PORTD |
| Button (active low) | RD2 (21) | PORTD |
| Sensitivity pot (wiper) | RA0 / AN0 (2) | PORTA |

Keep peripherals off **RB6 / RB7** — those are the PICkit's programming lines.

## How it works

- The servo angle is held in `servo_us`, scaled between `CLOSE_US` (0°) and `OPEN_US` (180°);
  `servo_pulse()` builds each pulse by hand, timed by Timer1.
- `read_cm()` fires a 10 µs trigger, times the echo on Timer1, and returns `ticks / 72`
  (the divisor for Timer1 at a 1:4 prescaler on 20 MHz — *not* the textbook 58).
- Each running frame reads the pot via `read_adc()` and maps the 0–1023 result to
  `detect_cm` (5–50 cm), so turning the knob changes the trigger distance immediately.
- The main loop runs one frame at a time: poll the button, then either hold (stopped)
  or run one sweep/detect step (running).

## Build & flash

1. Open the project in **MPLAB X IDE** with the **XC8** compiler.
2. Select device **PIC16F877A**.
3. Build, then program with a **PICkit** (use **MPLAB X IPE** if the standalone tool
   fails to load PICkit firmware).
4. `main.c` is the full turret. The `tests/` folder has standalone programs to bring up
   each part on its own before integrating.

## Repository layout

```
.
├── main.c                 # full turret firmware
├── README.md
├── .gitignore
└── tests/
    ├── chip_alive.c       # blink test: power + clock + programmer
    ├── servo_test.c       # servo sweep only (RB0)
    ├── sensor_test.c      # HC-SR04 -> laser indicator (RB3/RB4)
    ├── adc_test.c         # sensitivity pot on RA0 -> LED blink rate
    └── lcd_test.c         # optional 16x2 LCD test (PORTD, datasheet init)
```

## Notes / gotchas learned during the build

- **Servo power** is the #1 issue: a weak supply makes it judder instead of move. Use a
  dedicated 5 V source, common ground, and a 470 µF cap at the servo.
- **`cm = ticks / 72`**, not /58 — that factor matches Timer1 at a 1:4 prescaler on 20 MHz.
- **No interrupts** are used; the servo pulse is timed in a tight Timer1 loop, which keeps
  the motion smooth and never corrupts the echo measurement.
- Keep peripherals off **RB6/RB7** — those are the PICkit's programming lines.
- There's no display, so the sensitivity setting isn't shown anywhere: to verify the pot,
  set the knob, walk an object in, and note the distance it locks at; turn the knob and it
  should change.
