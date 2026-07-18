/*
  Arduino Mega -> Pixhawk Orange Cube Plus
  HIGHRES_IMU MAVLink transmitter with simulated values.

  Purpose:
  - Test Pixhawk TELEM2 MAVLink reception.
  - Confirm Mission Planner can display HIGHRES_IMU (#105).
  - Use controlled values before connecting a live sensor.

  Wiring:
  Pixhawk TELEM2 TX -> Arduino Mega RX1 / Pin 19
  Pixhawk TELEM2 RX -> Arduino Mega TX1 / Pin 18
  Pixhawk GND       -> Arduino Mega GND

  Mission Planner:
  SERIAL2_BAUD     = 57600
  SERIAL2_PROTOCOL = MAVLink1
*/

#include <MAVLink.h>

#define PIXHAWK_SERIAL Serial1
#define DEBUG_SERIAL   Serial

#define PIXHAWK_BAUD 57600
#define DEBUG_BAUD   9600

#define SYSTEM_ID    1
#define COMPONENT_ID 200

void setup() {
  PIXHAWK_SERIAL.begin(PIXHAWK_BAUD);
  DEBUG_SERIAL.begin(DEBUG_BAUD);

  delay(1000);
  DEBUG_SERIAL.println("Starting HIGHRES_IMU simulated transmitter...");
}

void loop() {
  sendHighresIMU();
  delay(100);  // 10 Hz
}

void sendHighresIMU() {
  uint64_t time_usec = micros();

  // HIGHRES_IMU units:
  // acceleration: m/s^2
  // gyroscope: rad/s
  // magnetometer: gauss
  // pressure: hPa
  // temperature: Celsius
  float xacc = 0.10f;
  float yacc = -0.20f;
  float zacc = 0.30f;

  float xgyro = 0.01f;
  float ygyro = -0.02f;
  float zgyro = 0.03f;

  float xmag = 0.001f;
  float ymag = -0.001f;
  float zmag = 0.002f;

  float abs_pressure = 1013.25f;
  float diff_pressure = 0.0f;
  float pressure_alt = 100.0f;
  float temperature = 25.0f;

  uint16_t fields_updated = 0xFFFF;
  uint8_t id = 1;

  mavlink_message_t msg;
  uint8_t buffer[MAVLINK_MAX_PACKET_LEN];

  mavlink_msg_highres_imu_pack(
    SYSTEM_ID,
    COMPONENT_ID,
    &msg,
    time_usec,
    xacc, yacc, zacc,
    xgyro, ygyro, zgyro,
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

  DEBUG_SERIAL.print("HIGHRES_IMU sent | bytes=");
  DEBUG_SERIAL.print(len);
  DEBUG_SERIAL.print(" | time_usec=");
  DEBUG_SERIAL.println((unsigned long)time_usec);
}
