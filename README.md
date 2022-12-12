# **Is the AC on?**
* Determine is the the ac is on in certain rooms with an ESP32 microcontroller and a temperature sensor

## Project Overview
* Utilizing an ESP32 and Temperature Sensor, this system will measure the temperature in a room and determine whether the AC in the room is on or off.
* The temperature measurements taken will be sent to an application set up in a server called NodeRED where it will the display them in a user interface.
* For sending and receiving data from ESP32 and vice-versa, we'll use an MQTT protocal.
* The system must support calibration via the user interface from NodeRED.
* We use the FreeRTOS library to handle multiple tasks in parallel to each other.

## Toolset
# Libraries:
* stdio.h
* PubSubClient.h
* driver/adc.h
* WiFi.h
* freertos/FreeRTOS.h
* math.h
# Hardware Components
* ESP32 Toolkit
* LM35Z Temperature Sensor

## Method Details
* connectToWiFi() -> Method for connecting to WiFi once the all required credentials are set up.
* setupMQTT() -> Sets the sever and port where the broker is. Also sets the callback function
* reconnect() -> Connects the client (ESP32) to the Aedes Broker and subscribes to topics.
* callback() -> Receives payloads from topics in NodeRED and filters them by topic.
* byteP_to_float() -> Turns byte values into float numbers. Used when receiving numeric payload from NodeRED.

## Participants
* Sebastian Ramirez sebastian.ramirez1@upr.edu
* Yadiel Galloza Rodriguez yadiel.galloza@upr.edu
* Ramón J. Rosario Recci ramon.rosario3@upr.edu
