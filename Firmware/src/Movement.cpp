#include <Arduino.h>

#include "Movement.h"
#include "config.h"

// This file is for motor and walking code.
// Right now, only a very small motor test is active.
// The more complicated walking and IMU balance ideas are saved below as commented-out future code.

namespace {

// Servos normally expect a control signal repeated about 50 times per second.
// That is why the PWM frequency is 50 Hz.
constexpr int SERVO_PWM_FREQUENCY_HZ = 50;

// PWM resolution controls how many tiny steps the ESP32 can use for the signal.
// 16 bits gives values from 0 to 65535, which is smooth enough for servo pulses.
constexpr int SERVO_PWM_RESOLUTION_BITS = 16;

// LEDC is the ESP32 PWM system.
// Each servo gets its own PWM channel, like a separate output lane.
constexpr int SERVO1_PWM_CHANNEL = 0;
constexpr int SERVO2_PWM_CHANNEL = 1;

// Servo pulses are measured in microseconds.
// For many servos:
// - 1500 us is center / stop / neutral
// - 1300 us moves one direction
// - 1700 us moves the other direction
// These are gentle test values. If the motor moves too far, use values closer to 1500.
constexpr int SERVO_LOW_TEST_PULSE_US = 1300;
constexpr int SERVO_CENTER_PULSE_US = 1500;
constexpr int SERVO_HIGH_TEST_PULSE_US = 1700;

// How long each simple motor test position is held.
// 1000 ms = 1 second.
constexpr unsigned long SIMPLE_MOTOR_TEST_HOLD_MS = 1000;

// Converts a servo pulse length, like 1500 us, into the duty value that ledcWrite() needs.
// The ESP32 LEDC function does not take microseconds directly, so this helper does the math.
uint32_t servoPulseToDuty(int pulseUs) {
  const uint32_t maxDuty = (1UL << SERVO_PWM_RESOLUTION_BITS) - 1;
  return (static_cast<uint64_t>(pulseUs) * maxDuty * SERVO_PWM_FREQUENCY_HZ) / 1000000ULL;
}

// Sends one servo pulse setting to one PWM channel.
// Example: writeServoPulse(SERVO1_PWM_CHANNEL, 1500) puts motor 1 at center.
void writeServoPulse(int channel, int pulseUs) {
  ledcWrite(channel, servoPulseToDuty(pulseUs));
}

// Puts both motors into their neutral position.
// This is the safest place to start and finish a motor test.
void centerMotors() {
  writeServoPulse(SERVO1_PWM_CHANNEL, SERVO_CENTER_PULSE_US);
  writeServoPulse(SERVO2_PWM_CHANNEL, SERVO_CENTER_PULSE_US);
}

// This is the only movement test that currently runs.
// It does not try to walk yet. It only checks that each motor responds.
void runSimpleMotorTest() {
  // Step 1: Start both motors at center so nothing suddenly moves at boot.
  Serial.println("Simple motor test: center M1 and M2.");
  centerMotors();
  delay(SIMPLE_MOTOR_TEST_HOLD_MS);

  // Step 2: Move only M1 while M2 stays centered.
  Serial.println("Simple motor test: move M1.");
  writeServoPulse(SERVO1_PWM_CHANNEL, SERVO_LOW_TEST_PULSE_US);
  writeServoPulse(SERVO2_PWM_CHANNEL, SERVO_CENTER_PULSE_US);
  delay(SIMPLE_MOTOR_TEST_HOLD_MS);

  // Step 3: Return both motors to center before testing the next motor.
  Serial.println("Simple motor test: center M1 and M2.");
  centerMotors();
  delay(SIMPLE_MOTOR_TEST_HOLD_MS);

  // Step 4: Move only M2 while M1 stays centered.
  Serial.println("Simple motor test: move M2.");
  writeServoPulse(SERVO1_PWM_CHANNEL, SERVO_CENTER_PULSE_US);
  writeServoPulse(SERVO2_PWM_CHANNEL, SERVO_HIGH_TEST_PULSE_US);
  delay(SIMPLE_MOTOR_TEST_HOLD_MS);

  // Step 5: Finish centered so the robot is not left trying to move.
  Serial.println("Simple motor test done. Holding center.");
  centerMotors();
}

}

/*
  Future walking sequence section.
  This is commented out while the simple motor test is active.

  The idea here is to make the motor values change over time:

    1300 -> 1500 -> 1700 -> 1500 -> repeat

  That is closer to walking than holding one fixed motor value forever.

  constexpr unsigned long BASIC_WALK_STEP_INTERVAL_MS = 1200;
  constexpr int BASIC_WALK_STEP_COUNT = 4;

  const int SERVO1_WALK_SEQUENCE_US[BASIC_WALK_STEP_COUNT] = {
    SERVO_LOW_TEST_PULSE_US,
    SERVO_CENTER_PULSE_US,
    SERVO_HIGH_TEST_PULSE_US,
    SERVO_CENTER_PULSE_US,
  };

  const int SERVO2_WALK_SEQUENCE_US[BASIC_WALK_STEP_COUNT] = {
    SERVO_HIGH_TEST_PULSE_US,
    SERVO_CENTER_PULSE_US,
    SERVO_LOW_TEST_PULSE_US,
    SERVO_CENTER_PULSE_US,
  };

  int currentWalkStep = 0;
  unsigned long lastWalkStepAt = 0;

  void writeBasicWalkStep(int stepIndex) {
    writeServoPulse(SERVO1_PWM_CHANNEL, SERVO1_WALK_SEQUENCE_US[stepIndex]);
    writeServoPulse(SERVO2_PWM_CHANNEL, SERVO2_WALK_SEQUENCE_US[stepIndex]);
  }

  void updateBasicWalking() {
    if (millis() - lastWalkStepAt < BASIC_WALK_STEP_INTERVAL_MS) {
      return;
    }

    lastWalkStepAt = millis();
    writeBasicWalkStep(currentWalkStep);
    currentWalkStep = (currentWalkStep + 1) % BASIC_WALK_STEP_COUNT;
  }
*/

/*
  Future IMU walking correction section.
  Keep this commented out until roll/pitch values are tested by hand.

  This future section has 3 layers:

    ISM330DHCX library -> complementary filter -> walking correction

  The IMU reads how CubeOS is tilted.
  The complementary filter turns raw accelerometer and gyro data into smoother roll/pitch angles.
  The walking correction would later use those angles to adjust motor pulses.

  #include <Wire.h>
  #include <Adafruit_ISM330DHCX.h>

  namespace {

  Adafruit_ISM330DHCX imu;

  float roll = 0.0;
  float pitch = 0.0;
  float rollCorrection = 0.0;
  float pitchCorrection = 0.0;

  unsigned long lastImuMicros = 0;

  constexpr float COMPLEMENTARY_FILTER_ALPHA = 0.98;
  constexpr float RAD_TO_DEG = 57.2957795;
  constexpr float TARGET_ROLL = 0.0;
  constexpr float TARGET_PITCH = 0.0;
  constexpr float ROLL_GAIN = 1.5;
  constexpr float PITCH_GAIN = 1.5;

  bool setupImu() {
    Wire.begin(I2C_SDA_PIN, I2C_SCL_PIN);

    if (!imu.begin_I2C()) {
      Serial.println("Failed to find ISM330DHCX");
      return false;
    }

    Serial.println("ISM330DHCX found");
    lastImuMicros = micros();
    return true;
  }

  void readImu(sensors_event_t& accel, sensors_event_t& gyro, sensors_event_t& temp) {
    imu.getEvent(&accel, &gyro, &temp);
  }

  float secondsSinceLastImuRead() {
    unsigned long now = micros();
    float dt = (now - lastImuMicros) / 1000000.0;
    lastImuMicros = now;
    return dt;
  }

  void updateComplementaryFilter(const sensors_event_t& accel, const sensors_event_t& gyro, float dt) {
    float accelRoll = atan2(accel.acceleration.y, accel.acceleration.z) * RAD_TO_DEG;
    float accelPitch = atan2(
      -accel.acceleration.x,
      sqrt(
        accel.acceleration.y * accel.acceleration.y +
        accel.acceleration.z * accel.acceleration.z
      )
    ) * RAD_TO_DEG;

    float gyroXDeg = gyro.gyro.x * RAD_TO_DEG;
    float gyroYDeg = gyro.gyro.y * RAD_TO_DEG;

    roll = COMPLEMENTARY_FILTER_ALPHA * (roll + gyroXDeg * dt) +
      (1.0 - COMPLEMENTARY_FILTER_ALPHA) * accelRoll;
    pitch = COMPLEMENTARY_FILTER_ALPHA * (pitch + gyroYDeg * dt) +
      (1.0 - COMPLEMENTARY_FILTER_ALPHA) * accelPitch;
  }

  void calculateBalanceCorrection() {
    rollCorrection = (TARGET_ROLL - roll) * ROLL_GAIN;
    pitchCorrection = (TARGET_PITCH - pitch) * PITCH_GAIN;
  }

  void printImuAngles() {
    Serial.print("Roll: ");
    Serial.print(roll);
    Serial.print(" | Pitch: ");
    Serial.println(pitch);
  }

  void walkWithCorrection() {
    float normalServo1PulseUs = SERVO_CENTER_PULSE_US;
    float normalServo2PulseUs = SERVO_CENTER_PULSE_US;

    float servo1PulseUs = normalServo1PulseUs + rollCorrection + pitchCorrection;
    float servo2PulseUs = normalServo2PulseUs - rollCorrection + pitchCorrection;

    writeServoPulse(SERVO1_PWM_CHANNEL, servo1PulseUs);
    writeServoPulse(SERVO2_PWM_CHANNEL, servo2PulseUs);
  }

  void updateImuWalkingCorrection() {
    sensors_event_t accel;
    sensors_event_t gyro;
    sensors_event_t temp;

    readImu(accel, gyro, temp);
    float dt = secondsSinceLastImuRead();
    updateComplementaryFilter(accel, gyro, dt);
    calculateBalanceCorrection();
    printImuAngles();

    // Only call this after roll and pitch behave correctly when CubeOS is tilted by hand.
    // walkWithCorrection();
  }

  }
*/

// setupMovement() is called once when the ESP32 boots.
// It prepares the PWM outputs, connects those outputs to the servo pins, and runs one simple test.
void setupMovement() {
  // Create the PWM signal generators for M1 and M2.
  ledcSetup(SERVO1_PWM_CHANNEL, SERVO_PWM_FREQUENCY_HZ, SERVO_PWM_RESOLUTION_BITS);
  ledcSetup(SERVO2_PWM_CHANNEL, SERVO_PWM_FREQUENCY_HZ, SERVO_PWM_RESOLUTION_BITS);

  // Connect the PWM channels to the real ESP32 GPIO pins from config.h.
  ledcAttachPin(MOVEMENT_SERVO1_PWM_PIN, SERVO1_PWM_CHANNEL);
  ledcAttachPin(MOVEMENT_SERVO2_PWM_PIN, SERVO2_PWM_CHANNEL);

  // Run the current beginner-safe test.
  runSimpleMotorTest();
}

// updateMovement() is called over and over from the main loop.
// It is empty for now because the active code only runs one motor test at startup.
void updateMovement() {}
