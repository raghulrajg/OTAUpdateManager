# OTAUpdateManager
Implementation of OTA for remote monitoring and control of IoT devices

[![ESP8266](https://img.shields.io/badge/ESP-8266-000000.svg?longCache=true&style=flat&colorA=CC101F)](https://www.espressif.com/en/products/socs/esp8266)

[![ESP32](https://img.shields.io/badge/ESP-32-000000.svg?longCache=true&style=flat&colorA=CC101F)](https://www.espressif.com/en/products/socs/esp32)
[![ESP32](https://img.shields.io/badge/ESP-32S2-000000.svg?longCache=true&style=flat&colorA=CC101F)](https://www.espressif.com/en/products/socs/esp32-s2)
[![ESP32](https://img.shields.io/badge/ESP-32C3-000000.svg?longCache=true&style=flat&colorA=CC101F)](https://www.espressif.com/en/products/socs/esp32-c3)


**This works with the ESP8266 Arduino platform**

[https://github.com/esp8266/Arduino](https://github.com/esp8266/Arduino)

**This works with the ESP32 Arduino platform** 

[https://github.com/espressif/arduino-esp32](https://github.com/espressif/arduino-esp32)


## Contents
 - [How it works](#how-it-works)
 - [Quick start](#quick-start)
   - Installing
     - [Arduino - Through Library Manager](#install-through-library-manager)
     - [Arduino - From Github](#checkout-from-github)
   - [Using](#using)
 - [Documentation](#documentation)
   - [Server connection configuration](#Server-connection-configuration)

## How It Works
- When your ESP (Espressif Systems' microcontroller) starts up, it attempts to connect to a previously saved Access Point (AP) in Station mode. If the ESP fails to connect (either because no previous network is saved or the connection attempt is unsuccessful), it switches to Access Point mode. In this mode, the ESP sets up a DNS server and a WebServer with a default IP address of 192.168.4.1.

### Initial Startup in Station Mode:
- The ESP tries to connect to a saved Access Point.
- If the connection is unsuccessful or no network is saved, the ESP switches to Access Point mode.
- The ESP creates its own Wi-Fi network.
- The default IP address for this network is 192.168.4.1.
- Any Wi-Fi enabled device (computer, phone, tablet) can connect to this network using a web browser.

### User Connection:
- Connect your device to the newly created Access Point.
- Manually enter the network name (SSID) and password.

### MQTT Connection:
- The ESP can connect to an MQTT server to check for updates and maintain a live connection.
- This connection allows for monitoring the live online state and managing updates.

### Web Application:
- Use the web application to check the live online state.
- Upload new code to the ESP board through the update section.

## How It Looks
![ESP8266 WiFi Captive Portal Homepage](https://i.imgur.com/3LIUSZR.png) ![ESP8266 WiFi Captive Portal Homepage](https://i.imgur.com/sccSpXp.png) ![ESP8266 WiFi Captive Portal Homepage](https://i.imgur.com/NzAXmK6.png)

## Quick Start

### Installing
You can either install through the Arduino Library Manager or checkout the latest changes or a release from github

#### Install through Library Manager

### Using
- Include in your sketch
```cpp
#include <OTAUpdateManager.h>          
```

- setup function add

- In APN mode, the OTAUpdateManager is initialized with the following parameters:
```cpp
OTAUpdateManager(User, Token, ApnOn, apName, apPassword);
```
- User: The userid for authentication.
- Token: The token for secure access.
- ApnOn: A flag indicating that APN mode is active.
- apName: The name of the Access Point.
- apPassword: The password for the Access Point


- In WiFi mode, the OTAUpdateManager is initialized with the following parameters:
```cpp
OTAUpdateManager(User, Token, WiFiOn, SSID, Password);
```
- User: The userid for authentication.
- Token: The token for secure access.
- ApnOn: A flag indicating that APN mode is active.
- SSID: The SSID of the WiFi network.
- Password: The password for the WiFi network.

- if we want to use default
```cpp
OTAUpdateManager(User, Token);
```
- User: The userid for authentication.
- Token: The token for secure access.

Also see [examples](https://github.com/raghulrajg/OTAUpdateManager/tree/master/examples).

## Documentation

### Server connection configuration

#### Connecting to the Web Application:
- Go to the website: https://ota.serveo.net/.
- Log in to the web application.

#### Token Configuration:
- In the Token section, a default token is created.
- Copy the UserID and Token from this section.

#### Example Code Integration:
- Paste the UserID and Token into your example code.
- Set your connection method to either manual WiFi or APN.

#### Upload Program:
- Upload the program to the ESP.
- The ESP will connect to the MQTT server and send packets to keep the connection alive.

### OTA Update Process

#### Update Detection:
- The server will notify the ESP when an update is available.
- The ESP receives a payload to prepare for the update.
 
#### Update Execution:
- Once the update process is initiated, the ESP will start updating.
- The update status can be monitored on the website in the status section.