## **Movement Monitoring and MQTT Integration**

### **Overview**
This script monitors movement data using a DFRobot Human Detection Sensor connected to an ESP8266 board. It sends the calculated running average of movement data to an MQTT broker for further processing. The logic includes thresholds for detecting significant changes in movement and ensuring robust communication via MQTT.
The Function Node Logic for it can be integrated with the file in this folder: "Node_RED_Function.txt"

### **Devices Involved**
1. **ESP8266 (e.g., NodeMCU):** The microcontroller with built-in Wi-Fi capabilities for MQTT communication.
2. **DFRobot Human Detection Sensor:** Measures heart rate, breathing rate, and movement data.
3. **MQTT Broker:** Receives movement data from the ESP8266 and processes or forwards it.

### **How It Works**
1. **Sensor Initialization:**
   - The script initializes the DFRobot Human Detection Sensor using `SoftwareSerial` for communication.
   - The ESP8266 communicates with the sensor via GPIO pins (default: GPIO4 and GPIO5, corresponding to D2 and D1).

2. **Wi-Fi Connection:**
   - The ESP8266 connects to a specified Wi-Fi network using the credentials defined in the script.

3. **Data Processing:**
   - Movement data is continuously read from the sensor.
   - A circular buffer stores the most recent readings, and a running average is calculated for every new entry.

4. **MQTT Communication:**
   - The running average is published to a specific MQTT topic (`/movement`) every 2 seconds.
   - MQTT reconnect logic ensures that the device remains connected to the broker.

5. **Threshold Logic (in Node-RED):**
   - Node-RED processes the MQTT messages:
     - Output 1 triggers `"on"` when the average movement exceeds `25` and `"off"` when it falls below `20`.
     - Output 2 triggers `"on"` if the average drops below `15` for at least 5 consecutive readings.

### **Adjustable Parameters**
- **Wi-Fi Credentials:**
  - Update `ssid` and `password` to match your Wi-Fi network.
- **MQTT Configuration:**
  - Set `mqtt_server`, `mqtt_port`, and `mqtt_topic` for your MQTT broker.
- **Threshold Logic (Node-RED):**
  - Thresholds and logic are defined in the Node-RED function node for flexibility.

### **Setup and Wiring**
- **Wiring:**
  - **Sensor TX → ESP GPIO4 (D2)**
  - **Sensor RX → ESP GPIO5 (D1)**
  - Ensure the sensor is properly powered and connected.

DFRobot Sensor                  ESP8266 (NodeMCU)
+-------------+                 +------------------+
|             |                 |                  |
|     VCC     +---------------->+     VIN (5V)     |
|     GND     +---------------->+      GND         |
|      TX     +---------------->+     GPIO4 (D2)   |
|      RX     +---------------->+     GPIO5 (D1)   |
+-------------+                 +------------------+



- **Software:**
  - Install the required libraries:
    - `ESP8266WiFi` (should come with the Board, else: https://github.com/esp8266/Arduino/tree/master/libraries)
    - `PubSubClient` (by Nick O'Leary: https://pubsubclient.knolleary.net/)
    - `SoftwareSerial` (https://docs.arduino.cc/learn/built-in-libraries/software-serial/)
    - `DFRobot_HumanDetection` (https://github.com/DFRobot/DFRobot_HumanDetection/tree/master)

### **Usage**
1. Load the script onto the ESP8266 using the Arduino IDE.
2. Power the ESP8266 and ensure it connects to the Wi-Fi network.
3. Monitor the `Serial Monitor` for debug output, including Wi-Fi status and MQTT messages.
4. Use Node-RED or another MQTT client to process data sent to the `/movement` topic.
