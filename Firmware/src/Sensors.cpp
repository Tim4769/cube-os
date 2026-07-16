#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_ISM330DHCX.h>

#include "Sensors.h"
#include "config.h"

namespace {

    constexpr uint8_t IMU_ADDRESS = 0x6A; // I2C address for ISM330DHCX
    constexpr unsigned long IMU_PRINT_INTERVAL_MS = 250;

    Adafruit_ISM330DHCX imu;

    bool imuReady = false;
    unsigned long lastImuPrintTime = 0;

    const char* i2cDeviceName(uint8_t address) {
        switch (address) {
            case 0x10:
                return "VEML7700 light sensor";
            case 0x20:
                return "TCA6408 GPIO expander";

            case 0x44:
                return "SHT45 temperature/humidity sensor";

            case 0x6A:
                return "ISM330DHCX IMU";

            default:
                return "Unknown device";
        }
    }
}

void scanI2CBus() {
    Serial.println("Scanning I2C bus for devices...");

    Serial.println("I2C devices found:");

    int deviceCount = 0;

    for (uint8_t address = 1; address < 127; address++) {
        Wire.beginTransmission(address);
        uint8_t result = Wire.endTransmission();

        if (result == 0) {
            Serial.print("Found 0x");
            Serial.print(address, HEX);
            Serial.print(": ");
            Serial.println(i2cDeviceName(address));
            deviceCount++;
        }
        delay(2); // Short delay between address checks
    }
    Serial.print("Total devices found: ");
    Serial.println(deviceCount);
}

void setupSensors() {
    Serial.println("Starting I2C bus...");

    bool started = Wire.begin(I2C_SDA_PIN, I2C_SCL_PIN, 100000);
    if (!started) {
        Serial.println("Failed to start I2C bus.");
        return;
    }
    delay(100); // Give some time for devices to power up

    scanI2CBus();

    Serial.println("Starting ISM330DHCX IMU...");

    if (!imu.begin_I2C(IMU_ADDRESS, &Wire)) {
        Serial.println("Failed to initialize ISM330DHCX");
        return;
    }

    // ±4 g is suitable for basic tilt and movement testing.
    imu.setAccelRange(LSM6DS_ACCEL_RANGE_4_G);
    // ±500 degrees per second is suitable for movement by hand.
    imu.setGyroRange(LSM6DS_GYRO_RANGE_500_DPS);

    // Read both sensors internally at 104 samples per second.
    imu.setAccelDataRate(LSM6DS_RATE_104_HZ);
    imu.setGyroDataRate(LSM6DS_RATE_104_HZ);

    imuReady = true;

    Serial.println("ISM330DHCX IMU initialized successfully.");
}

void updateSensors() {
    if (!imuReady) {
        return;
    }

    unsigned long now = millis();
    if (now - lastImuPrintTime < IMU_PRINT_INTERVAL_MS) {
        return;
    }

    lastImuPrintTime = now;
    sensors_event_t acceleration;
    sensors_event_t gyro;
    sensors_event_t temperature;

    if (!imu.getEvent(&acceleration, &gyro, &temperature)) {
        Serial.println("Failed to read from ISM330DHCX IMU.");
        return;
    }
    Serial.print("Accel m/s^2 — X: ");
    Serial.print(acceleration.acceleration.x, 2);
    Serial.print("  Y: ");
    Serial.print(acceleration.acceleration.y, 2);
    Serial.print("  Z: ");
    Serial.print(acceleration.acceleration.z, 2);

    Serial.print(" | Gyro rad/s — X: ");
    Serial.print(gyro.gyro.x, 2);
    Serial.print("  Y: ");
    Serial.print(gyro.gyro.y, 2);
    Serial.print("  Z: ");
    Serial.print(gyro.gyro.z, 2);

    Serial.print(" | Temperature: ");
    Serial.print(temperature.temperature, 2);
    Serial.println(" C");
}
