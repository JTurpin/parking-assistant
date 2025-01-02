# Garage Parking Assistant

An ESP32-based parking assistant that helps guide your car to the perfect parking position using a VL53L0X distance sensor and WS2811 LED strip. Integrates with Home Assistant via MQTT.

## Features
- Wall-mounted distance sensing using VL53L0X ToF sensor
- Visual feedback via WS2811 LED strip
- MQTT integration with Home Assistant
- Multiple states: Car Gone, Approaching, Parked, Too Close
- Real-time distance updates

## Hardware Requirements
- ESP32 development board
- VL53L0X Time-of-Flight distance sensor
- WS2811 LED strip
- 5V power supply
- Mounting hardware

## Wiring Diagram
          +----------------------+
          |       ESP32         |
          | 3.3V  --- VCC       | (to VL53L0X VCC)
          | GND   --- GND       | (shared ground)
          | GPIO21 --- SDA      | (to VL53L0X SDA)
          | GPIO22 --- SCL      | (to VL53L0X SCL)
          |                     |
          | 5V    --- VCC       | (to WS2811 LED strip VCC)
          | GND   --- GND       | (shared ground)
          | GPIO5  --- Data In  | (to WS2811 LED strip Data In)
          +----------------------+
                  
                  
                +---------------------+
VL53L0X Sensor: |       VL53L0X      |
                | VCC  <--- 3.3V     |
                | GND  <--- GND      |
                | SDA  <--- GPIO21   |
                | SCL  <--- GPIO22   |
                +---------------------+


                +---------------------+
WS2811 LED Strip |    WS2811 Strip    |
  (start/input): | Data In <--- GPIO5 |
                | VCC     <--- 5V     |
                | GND     <--- GND    |
                +---------------------+

Important Notes
Common Ground
All devices (ESP32, VL53L0X, and WS2811 strip) must share the same ground.

Power Requirements

The WS2811 LED strip typically runs on 5V.
Ensure your power supply can deliver sufficient current for the entire LED strip.
Data Level Shifting

Many WS2811 strips will accept the 3.3V signal directly from the ESP32.
If you encounter flickering or signal issues, consider adding a level shifter between the ESP32 data pin and the LED strip. You can also have a sacrificial WS2811 pixel close to the ESP32.

## Dependencies
- Arduino IDE
- Libraries:
  - WiFi.h
  - PubSubClient (MQTT)
  - Adafruit_VL53L0X
  - FastLED
  - ArduinoJson

## States
1. Car Gone (>2m): LEDs off
2. Car Approaching (2m - 1.3m): Blue moving pattern
3. Car Parked (1.3m - 0.7m): Solid Green
4. Too Close (<0.7m): Red flashing

## MQTT Topics
- `garage/parking/distance` - Current distance in meters
- `garage/parking/state` - Current state (GONE, APPROACHING, PARKED, TOO_CLOSE)
- `garage/parking/status` - Device status and connectivity

## Installation
1. Wire the components according to the wiring diagram
2. Install required libraries in Arduino IDE
3. Update WiFi and MQTT credentials in the code
4. Flash the ESP32
5. Mount the device on the wall
6. Add the provided configuration to Home Assistant

## Configuration
See the `config` directory for Home Assistant configuration examples.
