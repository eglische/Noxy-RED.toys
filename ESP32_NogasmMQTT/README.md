# ESP32 NOGASM for MQTT Implementation

This project implements a pressure monitoring system using an ESP32 with the following features:

## Features
- **Pressure Monitoring**: Reads pressure values from an ADS1115 ADC module.
- **Spike Detection**: Detects spikes in pressure based on sensitivity and gain settings.
- **MQTT Integration**:
  - Commands can be sent via MQTT to start/stop monitoring and adjust sensitivity or gain.
  - Status updates (e.g., "spike" or "cleared") are published to a status topic.
- **Serial Integration**:
  - Commands can also be sent via the serial interface.
  - Real-time pressure and average pressure readings are displayed.
- **EEPROM Persistence**:
  - Sensitivity and gain settings are saved to EEPROM for persistence across power cycles.
- **RTOS and Dual-Core Design**:
  - Core 0 handles pressure monitoring and detection.
  - Core 1 manages MQTT and serial communication.

## Hardware Requirements
- **ESP32 D1 NodeMCU**: Primary microcontroller.
- **ADS1115 ADC Module**: Analog-to-digital converter for pressure sensor readings.
- **Pressure Sensor**: Connected to the ADS1115. like a MPX5700AP

## MQTT Configuration
- **WiFi Credentials**:
  - SSID: `Your Wifi Name`
  - Password: `Your Wifi Password`
- **MQTT Broker**:
  - Server: `192.168.1.5` <- Enter here the IP of your MQTT Broker
  - Port: `1883`
- **MQTT Topics**:
  - **Command Topic**: `/sensorbt/command`
    - Commands include:
      - `start`: Begin monitoring.
      - `stop`: Stop monitoring.
      - `sensitivity:<value>`: Adjust sensitivity (e.g., `sensitivity:50`).
      - `gain:<value>`: Adjust gain (e.g., `gain:1.5`).
  - **Status Topic**: `/sensorbt/status`
    - Publishes status updates:
      - `spike`: When a spike is detected.
      - `cleared`: When the spike condition is resolved.

## Command Overview
- **Serial Commands**:
  - `start`: Starts monitoring.
  - `stop`: Stops monitoring.
  - `sensitivity:<value>`: Sets the sensitivity value. The Lower the value the more sensitive the detection will become
  - `gain:<value>`: Sets the gain value. Adjust this only if you have other Voltage than 3.3/3.7v
- **MQTT Commands**:
  - Same commands as serial.

## Key Parameters
- **Sensitivity**:
  - Determines the deviation required to detect a spike.
  - Lower values make the system more sensitive.
  - Example: Sensitivity of `50` triggers on smaller changes than `100`.
- **Gain**:
  - Multiplies the sensitivity threshold for finer adjustment.

## Workflow
1. **Setup**:
   - Connect the ESP32 to the pressure sensor via the ADS1115 ADC.
   - Connect to WiFi and MQTT broker.
2. **Monitoring**:
   - Start monitoring using MQTT or serial (`start` command).
   - Monitor pressure and average pressure via serial output.
   - Adjust sensitivity and gain as needed.
3. **Spike Detection**:
   - The system detects pressure spikes based on sensitivity and gain.
   - Publishes "spike" or "cleared" status to MQTT.

## Stabilization Period
- After starting, the system waits for 15 seconds to stabilize readings before detecting spikes.

## Additional Notes
- **Non-blocking Design**: Uses `millis()` for timing to ensure non-blocking operation.
- **EEPROM Use**: Persist sensitivity and gain settings to ensure they are retained across resets.
- **Error Handling**: Monitors ADS1115 initialization and MQTT connection status.



## ADS1115 to ESP32 Pin Connections
- VCC 3V3 Power supply for the ADS1115 (3.3V from the ESP32).
- GND GND Ground connection.
- SCL GPIO22 (SCL)  I2C Clock Line (ESP32 default SCL pin is GPIO22).
- SDA GPIO21 (SDA)  I2C Data Line (ESP32 default SDA pin is GPIO21).

## ADS1115 to MPX5700AP Pin Connections
- A0  Pressure Sensor Analog input for the pressure sensor (connect sensor output).
- GND and VCC either directly from the ESP to the Sensor, or from ADS1115

In Case of a bare MPX5700AP and not the Breakoutboard, you might want to add the recomended Capacitors to stabilize the Signal. Consult the Datasheet of the sensor for further Details
