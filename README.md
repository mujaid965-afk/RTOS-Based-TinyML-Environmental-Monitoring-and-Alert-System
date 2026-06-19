# RTOS-Based TinyML Environmental Monitoring and Alert System

## Overview

This project implements a real-time environmental monitoring and alert system using STM32, FreeRTOS, TinyML, and ESP32 IoT connectivity.

The system continuously monitors environmental parameters such as temperature, humidity, pressure, gas concentration, and vibration levels. Sensor data is processed by a TinyML-based anomaly detection model running on the STM32 microcontroller. Detected anomalies are transmitted to an ESP32 gateway, which uploads data to a cloud dashboard for remote monitoring.

---

## Features

* Real-time environmental monitoring
* FreeRTOS-based multitasking architecture
* TinyML anomaly detection on edge device
* UART communication between STM32 and ESP32
* Wi-Fi cloud connectivity using ESP32
* MQTT/HTTP cloud data publishing
* OLED display support
* Alert generation for abnormal conditions
* Modular and scalable embedded architecture

---

## System Architecture

```text
+------------------+
| Environmental    |
| Sensors          |
+--------+---------+
         |
         v
+------------------+
| STM32F401RE      |
| FreeRTOS         |
|                  |
| Sensor Task      |
| TinyML Task      |
| UART Task        |
| Alert Task       |
+--------+---------+
         |
     UART
         |
         v
+------------------+
| ESP32 Gateway    |
| Wi-Fi            |
| MQTT/HTTP        |
+--------+---------+
         |
         v
+------------------+
| Cloud Dashboard  |
| ThingSpeak       |
| Adafruit IO      |
| HiveMQ           |
+------------------+
```

---

## Hardware Components

### Main Controller

* STM32 Nucleo-F401RE

### IoT Gateway

* ESP32 Development Board

### Sensors

* BME280 (Temperature, Humidity, Pressure)
* MQ-2 Gas Sensor
* Vibration Sensor

### Display

* SSD1306 OLED Display (I2C)

### Alert Devices

* Passive Buzzer

### Miscellaneous

* Breadboard
* Jumper Wires
* USB Cables
* 5V Power Supply

---

## Software Stack

### STM32 Side

* STM32CubeIDE
* STM32 HAL Drivers
* FreeRTOS
* TensorFlow Lite Micro

### ESP32 Side

* ESP-IDF
* UART Driver
* Wi-Fi Stack
* MQTT/HTTP Client

### Cloud

* ThingSpeak
* Adafruit IO
* HiveMQ

---

## FreeRTOS Tasks

### Sensor Acquisition Task

Reads environmental sensor data periodically.

### TinyML Inference Task

Performs anomaly detection using trained TinyML model.

### UART Communication Task

Transfers processed data to ESP32 gateway.

### Alert Management Task

Activates alerts when anomalies are detected.

---

## Communication Protocol

### STM32 → ESP32 UART Packet

```text
Byte 1 : Temperature
Byte 2 : Gas Level
Byte 3 : Anomaly Status
```

Example:

```text
25,40,1
```

Where:

* Temperature = 25°C
* Gas Level = 40
* Anomaly Detected = Yes

---

## TinyML Workflow

1. Collect environmental sensor data.
2. Prepare dataset.
3. Train anomaly detection model.
4. Convert model to TensorFlow Lite format.
5. Deploy model using TensorFlow Lite Micro.
6. Run inference on STM32.
7. Generate alerts when abnormal patterns are detected.

---

## Project Structure

```text
RTOS_TinyML_EnvMonitor/
│
├── Core/
│   ├── Inc/
│   └── Src/
│
├── Drivers/
│
├── Middlewares/
│   └── FreeRTOS/
│
├── TinyML/
│   ├── model/
│   └── inference/
│
├── ESP32/
│   ├── main/
│   └── components/
│
├── Docs/
│
└── README.md
```

---

## Current Status

### Completed

* Project architecture finalized
* Hardware selection completed
* STM32CubeIDE project setup completed
* FreeRTOS integration completed
* UART communication framework developed
* Sensor interface planning completed
* ESP32 cloud architecture defined

### In Progress

* Sensor integration
* UART debugging
* ESP32 cloud upload implementation
* TinyML model deployment

### Planned

* End-to-end testing
* Dashboard visualization
* Performance optimization
* Final documentation

---

## Learning Outcomes

* Embedded Systems Design
* Real-Time Operating Systems (FreeRTOS)
* STM32 Development
* ESP32 IoT Development
* UART Communication
* TinyML Deployment
* Edge AI
* Cloud Integration
* MQTT Protocol

---

## Future Enhancements

* OTA firmware updates
* Mobile application integration
* Multiple node support
* LoRa communication
* Battery-powered deployment
* Advanced TinyML models

---

## Author

Muhammed Mujaid K

B.Tech Electronics and Communication Engineering

Government Engineering College Palakkad

Embedded Systems | RTOS | TinyML | IoT
