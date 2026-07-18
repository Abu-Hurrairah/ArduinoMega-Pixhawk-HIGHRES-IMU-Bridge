# Sensor-to-Pixhawk Communication Report  
## HIGHRES_IMU Data Transmission

## 1. Overview

This project demonstrates a MAVLink-based IMU telemetry bridge between an Arduino Mega and a Pixhawk Orange Cube Plus.

The Arduino Mega creates `HIGHRES_IMU` MAVLink messages and sends them to the Pixhawk through TELEM2. The stream can then be viewed in Mission Planner using Mavlink Inspector.

Two operating modes are included:

1. Simulated IMU values for controlled communication testing.
2. MPU9265 live sensor readings for an external IMU bridge workflow.

---

## 2. Communication Architecture

```text
MPU9265 / Simulated IMU Data
            ↓
      Arduino Mega
            ↓
 MAVLink HIGHRES_IMU Message
            ↓
 Pixhawk Orange Cube Plus
            ↓
 Mission Planner / Mavlink Inspector
```

The Pixhawk communication channel uses the TELEM2 port. The Arduino Mega uses `Serial1` at `57600` baud for MAVLink transmission and USB Serial at `9600` baud for debugging.

---

## 3. HIGHRES_IMU Message

`HIGHRES_IMU` is MAVLink message ID `105`. It can carry high-resolution sensor data including:

- acceleration values
- gyroscope values
- magnetometer values
- pressure values
- pressure altitude
- temperature
- timestamp
- field update mask

This format is useful when the system needs more flexible sensor telemetry than basic scaled IMU messages.

---

## 4. Hardware

| Hardware | Role |
|---|---|
| Arduino Mega | Reads/creates IMU data and sends MAVLink messages |
| Pixhawk Orange Cube Plus | Receives telemetry on TELEM2 |
| MPU9265 | External IMU sensor for live accelerometer and gyroscope values |
| Mission Planner | Used to inspect the MAVLink stream |

---

## 5. Wiring

| Pixhawk TELEM2 | Arduino Mega |
|---|---|
| TX | RX1 / Pin 19 |
| RX | TX1 / Pin 18 |
| GND | GND |

| MPU9265 | Arduino Mega |
|---|---|
| SDA | SDA / Pin 20 |
| SCL | SCL / Pin 21 |
| VCC | 3.3V / module-supported VCC |
| GND | GND |

---

## 6. Pixhawk Configuration

Recommended parameters:

| Parameter | Value |
|---|---:|
| `SERIAL2_BAUD` | `57600` |
| `SERIAL2_PROTOCOL` | `MAVLink1` |
| `AHRS_EKF_TYPE` | `3` |
| `EK3_ENABLE` | `1` |
| `EK3_IMU_MASK` | Based on selected IMU setup |

---

## 7. Code Versions

### 7.1 Simulated HIGHRES_IMU transmitter

The simulator sends fixed values for acceleration, gyro, magnetometer, pressure, altitude, temperature, and timestamp fields. This is useful for checking the telemetry pipeline before connecting live sensors.

File:

```text
src/Arduino_Mega_HIGHRES_IMU_Simulator.ino
```

### 7.2 MPU9265 HIGHRES_IMU bridge

The sensor bridge reads MPU9265 values through I²C, converts the readings, packs them into `HIGHRES_IMU`, and sends the serialized MAVLink frame to Pixhawk.

File:

```text
src/Arduino_Mega_MPU9265_HIGHRES_IMU_Bridge.ino
```

---

## 8. Mission Planner Run

The recorded Mission Planner session shows the `HIGHRES_IMU (#105)` stream visible in Mavlink Inspector, with IMU-related fields displayed while the Pixhawk connection is active.

The full video was converted into GitHub preview assets to keep the repository lightweight:

```text
assets/mission-planner-highres-imu-run.gif
assets/mission-planner-highres-imu-session.png
```

---

## 9. Limitations

- The simulator uses fixed values and should not be treated as live sensor data.
- The MPU9265 sketch currently focuses on accelerometer and gyroscope readings.
- Magnetometer, pressure, and temperature fields can be expanded as needed.
- Message visibility in Mission Planner depends on serial configuration and message routing.

---

## 10. Future Improvements

- Add magnetometer reading support from the MPU9265.
- Add barometer/temperature sensor support.
- Add configurable message rate.
- Add serial logging of MAVLink frame length and update rate.
- Add PC-side MAVLink listener for telemetry recording.
