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
          |       VL53L0X      |
          | VCC  <--- 3.3V     |
          | GND  <--- GND      |
          | SDA  <--- GPIO21   |
          | SCL  <--- GPIO22   |
          +---------------------+


          +---------------------+
          |    WS2811 Strip     | 
          | Data In <--- GPIO5  |
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
  - WiFiManager
  - PubSubClient (MQTT)
  - Adafruit_VL53L0X
  - FastLED
  - ArduinoJson

## Installation
1. Wire the components according to the wiring diagram
2. Install required libraries in Arduino IDE:
   - In Arduino IDE, go to Tools → Manage Libraries
   - Search for and install:
     - WiFiManager by tzapu
     - PubSubClient
     - Adafruit_VL53L0X
     - FastLED
     - ArduinoJson
3. Update MQTT credentials in the code
4. Flash the ESP32
5. Mount the device on the wall

## First-Time Setup
1. Power on the device
2. The LED strip will show a blue breathing pattern indicating AP (Access Point) mode
3. On your phone or computer, connect to the WiFi network named "ParkingAssistant-XXXXXX"
4. A configuration portal will automatically open (or navigate to 192.168.4.1)
5. Enter your configuration:
   - Select your WiFi network
   - Enter the WiFi password
   - Enter a unique name for your garage (e.g., "main_garage", "left_garage", etc.)
6. Click Save
7. The device will save these credentials and automatically reconnect on future power-ups

## LED Status Indicators
- Blue breathing pattern: WiFi configuration mode (AP mode)
- Yellow breathing pattern: MQTT disconnected
- Normal operation patterns:
  - Off: Car Gone
  - Moving blue: Car Approaching
  - Solid green: Car Parked
  - Flashing red: Too Close

## States
1. Car Gone (>2m): LEDs off
2. Car Approaching (2m - 1.3m): Blue moving pattern
3. Car Parked (1.3m - 0.7m): Solid Green
4. Too Close (<0.7m): Red flashing

## MQTT Topics
All MQTT topics are prefixed with your garage name:
- `homeassistant/{garage_name}/parking/distance` - Current distance in meters
- `homeassistant/{garage_name}/parking/state` - Current state (GONE, APPROACHING, PARKED, TOO_CLOSE)
- `homeassistant/{garage_name}/parking/status` - Device status and connectivity

## Configuration
See the `config` directory for Home Assistant configuration examples.
