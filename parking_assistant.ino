#include <WiFi.h>
#include <WiFiManager.h>
#include <PubSubClient.h>
#include <Wire.h>
#include <Adafruit_VL53L0X.h>
#include <FastLED.h>
#include <ArduinoJson.h>
#include <EEPROM.h>

// MQTT Settings
const char* mqtt_server = "YOUR_MQTT_SERVER";
const int mqtt_port = 1883;
const char* mqtt_user = "YOUR_MQTT_USER";
const char* mqtt_password = "YOUR_MQTT_PASSWORD";

// MQTT Topics - will be prefixed with garage name
char MQTT_DISTANCE_TOPIC[50];
char MQTT_STATE_TOPIC[50];
char MQTT_STATUS_TOPIC[50];

// Garage name storage
char garage_name[32] = "garage"; // Default name
#define EEPROM_SIZE 64
#define GARAGE_NAME_ADDRESS 0

// LED Strip
#define NUM_LEDS 30
#define LED_PIN 5
CRGB leds[NUM_LEDS];

// VL53L0X sensor
Adafruit_VL53L0X sensor = Adafruit_VL53L0X();

// State thresholds (in mm)
const int THRESHOLD_GONE = 2000;
const int THRESHOLD_APPROACHING = 1300;
const int THRESHOLD_PARKED = 700;

// State definitions
enum ParkingState {
  GONE,
  APPROACHING,
  PARKED,
  TOO_CLOSE
};

ParkingState currentState = GONE;
unsigned long lastMeasurement = 0;
const int MEASUREMENT_INTERVAL = 100; // 100ms between measurements

WiFiClient espClient;
PubSubClient client(espClient);

void loadGarageName() {
  EEPROM.begin(EEPROM_SIZE);
  if (EEPROM.read(GARAGE_NAME_ADDRESS) != 255) { // Check if EEPROM has been written to
    for (int i = 0; i < 32; i++) {
      garage_name[i] = EEPROM.read(GARAGE_NAME_ADDRESS + i);
      if (garage_name[i] == '\0') break;
    }
  }
  EEPROM.end();
  
  // Update MQTT topics with garage name
  snprintf(MQTT_DISTANCE_TOPIC, 50, "homeassistant/%s/parking/distance", garage_name);
  snprintf(MQTT_STATE_TOPIC, 50, "homeassistant/%s/parking/state", garage_name);
  snprintf(MQTT_STATUS_TOPIC, 50, "homeassistant/%s/parking/status", garage_name);
}

void saveGarageName(const char* name) {
  EEPROM.begin(EEPROM_SIZE);
  for (int i = 0; i < 32; i++) {
    EEPROM.write(GARAGE_NAME_ADDRESS + i, name[i]);
    if (name[i] == '\0') break;
  }
  EEPROM.commit();
  EEPROM.end();
  
  strncpy(garage_name, name, 31);
  garage_name[31] = '\0'; // Ensure null termination
  
  // Update MQTT topics with new garage name
  snprintf(MQTT_DISTANCE_TOPIC, 50, "homeassistant/%s/parking/distance", garage_name);
  snprintf(MQTT_STATE_TOPIC, 50, "homeassistant/%s/parking/state", garage_name);
  snprintf(MQTT_STATUS_TOPIC, 50, "homeassistant/%s/parking/status", garage_name);
}

void setup() {
  Serial.begin(115200);
  
  // Initialize LED strip
  FastLED.addLeds<WS2811, LED_PIN, GRB>(leds, NUM_LEDS);
  FastLED.setBrightness(64);
  
  // Initialize VL53L0X
  if (!sensor.begin()) {
    Serial.println("Failed to initialize VL53L0X!");
    while(1);
  }
  
  // Load saved garage name
  loadGarageName();
  
  // Setup WiFiManager
  WiFiManager wifiManager;
  
  // Add custom parameter for garage name
  WiFiManagerParameter custom_garage_name("garage_name", "Garage Name", garage_name, 31);
  wifiManager.addParameter(&custom_garage_name);
  
  // Set custom AP name
  String apName = "ParkingAssistant-" + String((uint32_t)ESP.getEfuseMac(), HEX);
  
  // Configure portal timeout
  wifiManager.setConfigPortalTimeout(180); // 3 minutes timeout
  
  // Set callback for when WiFi configuration fails and enters AP mode
  wifiManager.setAPCallback([](WiFiManager *myWiFiManager) {
    Serial.println("Entered config mode");
    Serial.println(WiFi.softAPIP());
    // Show blue breathing pattern on LEDs to indicate AP mode
    for(int i = 0; i < NUM_LEDS; i++) {
      leds[i] = CRGB::Blue;
      FastLED.setBrightness(((millis() / 20) % 128));
      FastLED.show();
    }
  });
  
  // Set save config callback
  wifiManager.setSaveConfigCallback([&custom_garage_name]() {
    Serial.println("Config saved");
    if (strlen(custom_garage_name.getValue()) > 0) {
      saveGarageName(custom_garage_name.getValue());
    }
  });
  
  // Try to connect to saved WiFi, if it fails, start config portal
  if (!wifiManager.autoConnect(apName.c_str())) {
    Serial.println("Failed to connect and hit timeout");
    // Reset and try again
    ESP.restart();
    delay(1000);
  }
  
  Serial.println("Connected to WiFi");
  Serial.print("Garage Name: ");
  Serial.println(garage_name);
  
  // Setup MQTT
  client.setServer(mqtt_server, mqtt_port);
}

void reconnect() {
  static unsigned long lastReconnectAttempt = 0;
  unsigned long now = millis();
  
  // Try to reconnect every 5 seconds
  if (now - lastReconnectAttempt > 5000) {
    lastReconnectAttempt = now;
    
    if (!client.connected()) {
      Serial.print("Attempting MQTT connection...");
      String clientId = "ESP32Parking-";
      clientId += String(random(0xffff), HEX);
      
      if (client.connect(clientId.c_str(), mqtt_user, mqtt_password)) {
        Serial.println("connected");
        publishStatus("online");
      } else {
        Serial.print("failed, rc=");
        Serial.print(client.state());
        Serial.println(" retrying in 5 seconds");
        
        // Show yellow breathing pattern to indicate MQTT disconnection
        for(int i = 0; i < NUM_LEDS; i++) {
          leds[i] = CRGB::Yellow;
          FastLED.setBrightness(((millis() / 20) % 128));
          FastLED.show();
        }
      }
    }
  }
}

void publishStatus(const char* status) {
  StaticJsonDocument<200> doc;
  doc["status"] = status;
  char buffer[200];
  serializeJson(doc, buffer);
  client.publish(MQTT_STATUS_TOPIC, buffer, true);
}

void updateLEDs(ParkingState state, int distance) {
  switch(state) {
    case GONE:
      fill_solid(leds, NUM_LEDS, CRGB::Black);
      break;
      
    case APPROACHING:
      // Blue moving pattern
      fadeToBlackBy(leds, NUM_LEDS, 20);
      int pos = (millis() / 100) % NUM_LEDS;
      leds[pos] = CRGB::Blue;
      break;
      
    case PARKED:
      fill_solid(leds, NUM_LEDS, CRGB::Green);
      break;
      
    case TOO_CLOSE:
      // Red flashing
      fill_solid(leds, NUM_LEDS, ((millis() / 500) % 2) ? CRGB::Red : CRGB::Black);
      break;
  }
  FastLED.show();
}

ParkingState getStateFromDistance(int distance) {
  if (distance > THRESHOLD_GONE) return GONE;
  if (distance > THRESHOLD_APPROACHING) return APPROACHING;
  if (distance > THRESHOLD_PARKED) return PARKED;
  return TOO_CLOSE;
}

void publishDistance(int distance) {
  char buffer[10];
  dtostrf(distance / 1000.0, 4, 2, buffer); // Convert to meters
  client.publish(MQTT_DISTANCE_TOPIC, buffer);
}

void publishState(ParkingState state) {
  const char* states[] = {"GONE", "APPROACHING", "PARKED", "TOO_CLOSE"};
  client.publish(MQTT_STATE_TOPIC, states[state]);
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  unsigned long currentMillis = millis();
  if (currentMillis - lastMeasurement >= MEASUREMENT_INTERVAL) {
    VL53L0X_RangingMeasurementData_t measure;
    sensor.rangingTest(&measure, false);

    if (measure.RangeStatus != 4) {  // Valid measurement
      int distance = measure.RangeMilliMeter;
      ParkingState newState = getStateFromDistance(distance);
      
      if (newState != currentState) {
        currentState = newState;
        publishState(currentState);
      }
      
      publishDistance(distance);
      updateLEDs(currentState, distance);
    }
    
    lastMeasurement = currentMillis;
  }
}
