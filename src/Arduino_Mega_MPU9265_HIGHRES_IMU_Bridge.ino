/*
  Arduino Mega + MPU9265 -> Pixhawk Orange Cube Plus
  HIGHRES_IMU MAVLink bridge.

  This sketch reads accelerometer and gyroscope values from an MPU9265 over I2C,
  converts them into HIGHRES_IMU-friendly units, packs the data into a MAVLink
  HIGHRES_IMU message, and sends the message to Pixhawk through TELEM2.

  Wiring:
  MPU9265 SDA -> Arduino Mega SDA / Pin 20
  MPU9265 SCL -> Arduino Mega SCL / Pin 21
  MPU9265 VCC -> 3.3V or module-supported VCC
  MPU9265 GND -> Arduino GND

  Pixhawk TELEM2 TX -> Arduino Mega RX1 / Pin 19
  Pixhawk TELEM2 RX -> Arduino Mega TX1 / Pin 18
  Pixhawk GND       -> Arduino Mega GND
*/

#include <Wire.h>
#include <MAVLink.h>

#define PIXHAWK_SERIAL Serial1
#define DEBUG_SERIAL   Serial

#define PIXHAWK_BAUD 57600
#define DEBUG_BAUD   9600

#define MPU9265_ADDR 0x68

#define SYSTEM_ID    1
#define COMPONENT_ID 200

const float G_TO_MS2 = 9.80665f;
const float DEG_TO_RAD_CUSTOM = 0.01745329251f;

float AccX = 0.0f;
float AccY = 0.0f;
float AccZ = 0.0f;

float GyroX = 0.0f;
float GyroY = 0.0f;
float GyroZ = 0.0f;

void setup() {
  PIXHAWK_SERIAL.begin(PIXHAWK_BAUD);
  DEBUG_SERIAL.begin(DEBUG_BAUD);
  Wire.begin();

  wakeMPU9265();

  delay(1000);
  DEBUG_SERIAL.println("MPU9265 initialized. Sending HIGHRES_IMU data...");
}

void loop() {
  readMPU9265();
  sendHighresIMU();
  delay(100);  // 10 Hz
}

void wakeMPU9265() {
  Wire.beginTransmission(MPU9265_ADDR);
  Wire.write(0x6B);   // PWR_MGMT_1 register
  Wire.write(0x00);   // wake sensor
  Wire.endTransmission();
}

void readMPU9265() {
  Wire.beginTransmission(MPU9265_ADDR);
  Wire.write(0x3B);  // accelerometer data start register
  Wire.endTransmission(false);

  Wire.requestFrom(MPU9265_ADDR, 14, true);

  int16_t rawAccX = Wire.read() << 8 | Wire.read();
  int16_t rawAccY = Wire.read() << 8 | Wire.read();
  int16_t rawAccZ = Wire.read() << 8 | Wire.read();

  // Skip raw temperature bytes in this minimal bridge.
  Wire.read();
  Wire.read();

  int16_t rawGyroX = Wire.read() << 8 | Wire.read();
  int16_t rawGyroY = Wire.read() << 8 | Wire.read();
  int16_t rawGyroZ = Wire.read() << 8 | Wire.read();

  // MPU9265 typical default scaling:
  // Accelerometer ±2g: 16384 LSB/g.
  // Gyroscope ±250 deg/s: 131 LSB/(deg/s).
  float accX_g = rawAccX / 16384.0f;
  float accY_g = rawAccY / 16384.0f;
  float accZ_g = rawAccZ / 16384.0f;

  float gyroX_dps = rawGyroX / 131.0f;
  float gyroY_dps = rawGyroY / 131.0f;
  float gyroZ_dps = rawGyroZ / 131.0f;

  // HIGHRES_IMU expects m/s^2 and rad/s.
  AccX = accX_g * G_TO_MS2;
  AccY = accY_g * G_TO_MS2;
  AccZ = accZ_g * G_TO_MS2;

  GyroX = gyroX_dps * DEG_TO_RAD_CUSTOM;
  GyroY = gyroY_dps * DEG_TO_RAD_CUSTOM;
  GyroZ = gyroZ_dps * DEG_TO_RAD_CUSTOM;
}

void sendHighresIMU() {
  uint64_t time_usec = micros();

  // Magnetometer, pressure, pressure altitude and temperature are kept at zero
  // in this base bridge. Extend them if your final sensor stack includes those readings.
  float xmag = 0.0f;
  float ymag = 0.0f;
  float zmag = 0.0f;
  float abs_pressure = 0.0f;
  float diff_pressure = 0.0f;
  float pressure_alt = 0.0f;
  float temperature = 0.0f;

  uint16_t fields_updated = 0x0FFF;
  uint8_t id = 0;

  mavlink_message_t msg;
  uint8_t buffer[MAVLINK_MAX_PACKET_LEN];

  mavlink_msg_highres_imu_pack(
    SYSTEM_ID,
    COMPONENT_ID,
    &msg,
    time_usec,
    AccX, AccY, AccZ,
    GyroX, GyroY, GyroZ,
    xmag, ymag, zmag,
    abs_pressure,
    diff_pressure,
    pressure_alt,
    temperature,
    fields_updated,
    id
  );

  uint16_t len = mavlink_msg_to_send_buffer(buffer, &msg);
  PIXHAWK_SERIAL.write(buffer, len);

  DEBUG_SERIAL.print("HIGHRES_IMU sent | AccX=");
  DEBUG_SERIAL.print(AccX, 4);
  DEBUG_SERIAL.print(" m/s^2 | GyroX=");
  DEBUG_SERIAL.print(GyroX, 4);
  DEBUG_SERIAL.println(" rad/s");
}
