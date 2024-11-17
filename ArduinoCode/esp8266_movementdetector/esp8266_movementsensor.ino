#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <SoftwareSerial.h>
#include "DFRobot_HumanDetection.h"

// WiFi credentials
const char* ssid = "YOURSSID";
const char* password = "YOURPASSWORD";

// MQTT broker details
const char* mqtt_server = "IP_OF_YOUR_MACHINE";
const int mqtt_port = 1883;
const char* mqtt_topic = "/movement";

WiFiClient espClient;
PubSubClient client(espClient);

// Define SoftwareSerial for communication with the sensor
SoftwareSerial sensorSerial(4, 5); // RX, TX pins

// Pass the SoftwareSerial instance to the sensor class
DFRobot_HumanDetection hu(&sensorSerial);

// Variables to hold sensor data
float heart_rate = 0;
float breathing_rate = 0;
float body_movement = 0;

// Buffer for body movement data over the last 5 seconds
#define BUFFER_SIZE 6 // 5 seconds * 3 readings per second
float movementBuffer[BUFFER_SIZE];
int bufferIndex = 0;
float movementSum = 0;

// Function to update running average using a circular buffer
void updateMovementBuffer(float newValue) {
  movementSum -= movementBuffer[bufferIndex];
  movementBuffer[bufferIndex] = newValue;
  movementSum += newValue;
  bufferIndex = (bufferIndex + 1) % BUFFER_SIZE;
}

float calculateAverage() {
  return movementSum / BUFFER_SIZE;
}

// Function to connect to WiFi
void setupWiFi() {
  delay(10);
  Serial.println("Connecting to WiFi...");
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nWiFi connected.");
}

// Function to connect to MQTT broker
void reconnectMQTT() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");

    // Attempt to connect
    if (client.connect("HumanDetectionClient")) {
      Serial.println("connected");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}

void setup() {
  // Initialize Serial for debugging
  Serial.begin(115200);

  // Initialize the SoftwareSerial for the sensor
  sensorSerial.begin(115200);

  // Initialize WiFi and MQTT
  setupWiFi();
  client.setServer(mqtt_server, mqtt_port);

  Serial.println("Start sensor initialization...");

  // Initialize the sensor
  while (hu.begin() != 0) {
    Serial.println("Sensor initialization failed! Retrying...");
    delay(1000);
  }

  Serial.println("Sensor initialization successful!");

  // Set the sensor to work in the sleep detection mode
  while (hu.configWorkMode(hu.eSleepMode) != 0) {
    Serial.println("Error switching to sleep detection mode.");
    delay(1000);
  }

  Serial.println("Switched to sleep detection mode.");

  // Initialize movement buffer
  for (int i = 0; i < BUFFER_SIZE; i++) {
    movementBuffer[i] = 0;
  }
}

void loop() {
  // Reconnect to MQTT if disconnected
  if (!client.connected()) {
    reconnectMQTT();
  }
  client.loop();

  // Fetch sensor data
  heart_rate = hu.gitHeartRate();
  breathing_rate = hu.getBreatheValue();
  body_movement = hu.smHumanData(hu.eHumanMovingRange);

  // Update movement buffer
  updateMovementBuffer(body_movement);

  // Calculate running average over the last 5 seconds
  float avgMovement = calculateAverage();

  // Publish the running average every 2 seconds
  static unsigned long lastPublishTime = 0;
  if (millis() - lastPublishTime >= 2000) {
    lastPublishTime = millis();

    String payload = String(avgMovement);
    client.publish(mqtt_topic, payload.c_str());

    Serial.print("Published to MQTT: ");
    Serial.println(payload);
  }

  // Print raw sensor data for debugging
  Serial.print("Movement: ");
  Serial.print(body_movement);
  Serial.print(" | Heart Rate: ");
  Serial.print(heart_rate);
  Serial.print(" BPM | Respiration Rate: ");
  Serial.print(breathing_rate);
  Serial.println(" breaths/min");

  // Delay to achieve ~3 readings per second
  delay(333);
}
