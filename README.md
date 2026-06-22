# Sunny — Autonomous Obstacle-Avoiding 4WD Robot

**Status:** Active build — pivoting from full autonomy/line-following into a focused PID steering project.

Sunny is an autonomous robotics platform developed from scratch using an Arduino Uno and a collection of sensors, actuators, and communication interfaces. The primary objective of the project was to gain hands-on experience with robotics fundamentals including motor control, sensor integration, object avoidance, communication protocols, control systems, and software architecture.

## Currently Working

- **4WD Motor Control** — forward / backward / left / right, plus custom-timed precision turns (DRV8835 motor driver)
- **Ultrasonic Obstacle Avoidance** — HC-SR04 mounted on a servo gimbal sweeps 0°–180°, finds the clearest open direction, and turns the bot that way
- **MPU-6050 Gyroscope Integration** — I2C communication, a live calibration routine to zero out drift, and the math to convert raw gyro data into precise turn angles
- **Class Based Programing Architecture** - Modular object-oriented software architecture

## Code Architecture

| Class | Job |
|---|---|
| `Motor_Control` | Low-level 4WD driving + degree-based turning |
| `Sensors` | Ultrasonic distance reading + smoothing/averaging |
| `Obstacle_Avoidance` | Servo sweep + "find the open path" decision logic |
| `MPU650_MotionTracking` | I2C comms with the gyro, calibration, live motion data |

## Roadmap

- **Bluetooth UART link** (bot ↔ computer) to watch live gyro output — in progress
- **PID controller** using gyro + accelerometer data to correct steering and tighten up turning accuracy — current main focus
- **Line following + ESP32 camera FPV tracking** — shelved for now, may return once the PID core is solid
- **ML-based perception** — Machine learning and perception integration for making interactions with it's environment smarter - future stretch goal once the control loop is dialed in

## Parts List

**Core Electronics**
- ELEGOO UNO R3 — main controller
- V4.0 Expansion/IO Shield — 54-port shield w/ built-in IR receiver + Upload/Cam toggle
- ESP32-WROVER Camera Module (OV2640, 1024×768) — Wi-Fi FPV video
- GY-521 MPU-6050 — 3-axis gyroscope + accelerometer

**Sensors & Modules**
- HC-SR04 Ultrasonic Sensor — servo-mounted for distance scanning
- 3-channel IR Line-Tracking Module — chassis-mounted (for the line-following mode)
- IR Remote Controller — manual override

**Mechanics, Motors & Power**
- 4x DC Gear Motors (1:48 ratio) — 4WD drive
- SG90 Servo — rotates the ultrasonic sensor gimbal
- 4x rubber tires w/ plastic rims
- 7.4V 2000mAh LiPo pack — built-in switch, USB charging, low-battery LED

## Motivation
The purpose of this project was to develop a deeper understanding of robotics by building a complete autonomous system from the ground up rather than relying on pre-built software libraries or frameworks.

This project provided practical experience in:

- Embedded software development
- Sensor integration
- I2C communication
- UART communication
- Object-oriented programming in C++
- Robot navigation
- Environmental perception
- Control systems

The long-term goal is to use this platform as a testbed for more advanced robotics concepts including computer vision, machine learning, and autonomous navigation.

## 💡 Notes From the Build

- Getting the MPU-6050 talking over I2C was the real learning curve here. Each read comes back as a 16-bit packet — the first 8 bits are the register address, the second 8 bits are the actual data — and figuring out that bit/byte mapping was the "click" moment that made the rest of the gyro code make sense.
- Establishing communcation with the ESP32-WROVER Camera Module has come with challenges of its own. It is a third party board from a lesser known company, so configuring my computer to be able to interact with it took quite a while. Once my computer was able to detect the devices driver, then figuring out how to upload code to it was the next issue. I found out, that I essentially have to flash the device manually while the code was being uploaded, in order for the device to receive it. 
