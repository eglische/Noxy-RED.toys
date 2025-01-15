//=======Libraries===============================
#include <Wire.h>
#include <Adafruit_ADS1X15.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <EEPROM.h>

//=======WiFi and MQTT===========================
const char* ssid = "CHANGETHISVALUE";                             //enter your valid ssid
const char* password = "CHANGETHISVALUE";                         //enter your valid password
const char* mqtt_server = "192.168.1.5";                          //adjust to your mqtt broker
const int mqtt_port = 1883;
WiFiClient espClient;
PubSubClient client(espClient);
String clientId = "ESP32Client-" + String(random(0xffff), HEX);

// MQTT Topics
#define COMMAND_TOPIC "/sensorbt/command"
#define STATUS_TOPIC "/sensorbt/status"

//=======Hardware Setup==========================
#define ENC_SW 5  // Not used but kept for possible extension
#define ENC_SW_UP HIGH
#define ENC_SW_DOWN LOW

//ADS1115
Adafruit_ADS1115 ads;

//=======Software Settings=======================
#define FREQUENCY 60 // Update frequency in Hz
#define OVERSAMPLE 4
#define ADC_MAX 32767 // 16-bit ADC max value
#define STABILIZATION_TIME 15000 // 15 seconds
#define LONG_PRESS_MS 600

//Timing and sensitivity settings
#define DEFAULT_SENSITIVITY 50
#define DEFAULT_GAIN 1.0

float sensitivity = DEFAULT_SENSITIVITY;
float gain = DEFAULT_GAIN;

int pressure = 0;
int avgPressure = 0;
bool monitoring = false;
bool stabilized = false;
unsigned long startMonitoringTime = 0;

// EEPROM Addresses
#define SENSITIVITY_ADDR 0
#define GAIN_ADDR 4

//=======Setup===================================
void setup() {
  Serial.begin(115200);

  // Initialize EEPROM
  EEPROM.begin(8);
  sensitivity = EEPROM.readFloat(SENSITIVITY_ADDR);
  gain = EEPROM.readFloat(GAIN_ADDR);
  if (isnan(sensitivity)) sensitivity = DEFAULT_SENSITIVITY;
  if (isnan(gain)) gain = DEFAULT_GAIN;

  // Initialize ADS1115
  if (!ads.begin()) {
    Serial.println("Failed to initialize ADS1115. Check wiring!");
    while (1);
  }

  // Configure ADS1115 gain
  ads.setGain(GAIN_ONE);

  // Initialize WiFi
  connectWiFi();

  // Initialize MQTT
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(mqttCallback);

  Serial.println("System Ready. Send MQTT or serial commands to control.");
}

void connectWiFi() {
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected.");
}

void mqttReconnect() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    if (client.connect(clientId.c_str())) {
      Serial.println("connected");
      client.subscribe(COMMAND_TOPIC, 1);
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}

void mqttCallback(char* topic, byte* payload, unsigned int length) {
  String message = "";
  for (int i = 0; i < length; i++) {
    message += (char)payload[i];
  }
  message.trim();

  if (message == "start") {
    monitoring = true;
    stabilized = false;
    startMonitoringTime = millis();
    Serial.println("Monitoring started.");
  } else if (message == "stop") {
    monitoring = false;
    Serial.println("Monitoring stopped.");
  } else if (message.startsWith("sensitivity:")) {
    sensitivity = message.substring(12).toFloat();
    EEPROM.writeFloat(SENSITIVITY_ADDR, sensitivity);
    EEPROM.commit();
    Serial.print("Sensitivity set to: ");
    Serial.println(sensitivity);
  } else if (message.startsWith("gain:")) {
    gain = message.substring(5).toFloat();
    EEPROM.writeFloat(GAIN_ADDR, gain);
    EEPROM.commit();
    Serial.print("Gain set to: ");
    Serial.println(gain);
  } else {
    Serial.println("Invalid command.");
  }
}

//=======Main Logic==============================
void loop() {
  if (!client.connected()) {
    mqttReconnect();
  }
  client.loop();

  if (monitoring) {
    if (!stabilized) {
      if (millis() - startMonitoringTime > STABILIZATION_TIME) {
        stabilized = true;
        Serial.println("Monitoring stabilized.");
      }
    } else {
      readPressureAndDetect();
    }
    Serial.print("Pressure: ");
    Serial.print(pressure);
    Serial.print(", Avg Pressure: ");
    Serial.println(avgPressure);
  }

  handleSerialCommands();
}

void readPressureAndDetect() {
  pressure = readPressure();
  avgPressure = calculateRunningAverage(pressure);

  if (pressure - avgPressure > sensitivity * gain) {
    Serial.println("Spike detected.");
    client.publish(STATUS_TOPIC, "spike", true);
    waitForClear();
  }
}

int readPressure() {
  int raw = 0;
  for (int i = 0; i < OVERSAMPLE; i++) {
    raw += ads.readADC_SingleEnded(0);
  }
  return raw / OVERSAMPLE;
}

int calculateRunningAverage(int newSample) {
  static float total = 0;
  static int count = 0;

  count++;
  total += newSample;

  if (count > FREQUENCY) {
    total -= total / count; // Decay average over time
    count--;
  }

  return total / count;
}

void waitForClear() {
  while (pressure - avgPressure > sensitivity * gain) {
    pressure = readPressure();
    avgPressure = calculateRunningAverage(pressure);
    delay(100);
  }
  Serial.println("Cleared.");
  client.publish(STATUS_TOPIC, "cleared", true);
}

void handleSerialCommands() {
  if (Serial.available()) {
    String command = Serial.readStringUntil('\n');
    command.trim();

    if (command == "start") {
      monitoring = true;
      stabilized = false;
      startMonitoringTime = millis();
      Serial.println("Monitoring started.");
    } else if (command == "stop") {
      monitoring = false;
      Serial.println("Monitoring stopped.");
    } else if (command.startsWith("sensitivity:")) {
      sensitivity = command.substring(12).toFloat();
      EEPROM.writeFloat(SENSITIVITY_ADDR, sensitivity);
      EEPROM.commit();
      Serial.print("Sensitivity set to: ");
      Serial.println(sensitivity);
    } else if (command.startsWith("gain:")) {
      gain = command.substring(5).toFloat();
      EEPROM.writeFloat(GAIN_ADDR, gain);
      EEPROM.commit();
      Serial.print("Gain set to: ");
      Serial.println(gain);
    } else {
      Serial.println("Invalid command.");
    }
  }
}
