# Spider Robot — FreeRTOS-Controlled Quadruped

A 4-legged (12-DOF) walking robot built around an ESP32, with the full mechanical structure designed from scratch in SolidWorks — not a pre-made GrabCAD model. Each leg runs as an independent FreeRTOS task, giving the robot concurrent, non-blocking control over all four legs from a single microcontroller.


<img width="713" height="530" alt="Spider pic" src="https://github.com/user-attachments/assets/10f9af98-14c3-44d5-bfda-375babde9e10" />




https://github.com/user-attachments/assets/da0e5ce1-07fc-4307-80b6-2c7e29078677

## Mechanical Design

The chassis, leg links, and head were modelled entirely in SolidWorks as an original design — full part files, the top-level assembly, and 3D-printable STLs are included in `Spider Solidworks/`. Each leg has 3 degrees of freedom (hip, knee, ankle), for 12 servo-driven joints total.

## Embedded Architecture

The robot is built on an ESP32 (`esp32doit-devkit-v1`) using FreeRTOS as the real-time scheduler, with all 12 servos driven through a PCA9685 PWM driver over I2C.

**Per-leg concurrency.** Each leg is its own FreeRTOS task (`Leg_1`–`Leg_4`), created with `xTaskCreate` at equal priority and left running an infinite loop. There's no central gait-sequencer dictating timing to the legs — each task independently reads a shared `movement_states` struct (forward / backward / left / right) and drives its own 3-joint motion sequence. Walking gait emerges from staggering each task's startup delay (0 ms, 600 ms, 900 ms, 300 ms), which phase-offsets the legs relative to each other.

**Non-blocking timing.** Every wait in the motion code uses `vTaskDelay()` rather than a blocking `delay()`, so the scheduler can interleave all four leg tasks on the single core without one leg stalling the others.

**Low-level PWM driver.** The PCA9685 driver is written directly against the I2C register map (mode/prescale/PWM registers) rather than using an off-the-shelf servo library — `pca9685_set_pwm_freq`, `pca9685_set_pwm`, and `set_servo_angle` handle frequency configuration and per-channel pulse-width control for all 12 channels.

**Motion primitives.** `angle_motion()` steps three joint angles toward a target simultaneously, and `move_spider_leg()` chains three of these calls (lift, swing, plant) into one leg-stride cycle, parameterized by direction sign and step delay.

**App control (currently disabled).** The firmware includes Blynk/Wi-Fi integration (`BLYNK_WRITE` handlers mapped to the same four movement flags) for phone-app control, but this path is commented out in the current build — the robot currently runs from the `movement_states` flags directly rather than over Wi-Fi.

## Tech Stack

- **MCU:** ESP32 (Arduino framework via PlatformIO)
- **RTOS:** FreeRTOS (per-leg tasks, `vTaskDelay`-based scheduling)
- **Actuation:** 12x servo motors via PCA9685 PWM driver (I2C, custom register-level driver)
- **Connectivity:** Blynk/Wi-Fi (implemented, currently disabled in firmware)
- **Mechanical design:** SolidWorks (original design), 3D-printed structure

## Repository Structure

```
Spider Code/        PlatformIO project - ESP32 firmware
  src/main.cpp       Entry point, Blynk handlers, movement_states
  src/main.c          Servo driver, gait logic, FreeRTOS leg tasks
  src/movement_states.h

Spider Solidworks/  Original CAD - parts, assembly, and STLs for 3D printing
```

## Status

This was a personal hardware project - full design-to-deployment cycle from CAD through 3D printing to embedded firmware on real hardware.
