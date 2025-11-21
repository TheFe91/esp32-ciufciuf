# ESP32 CiufCiuf - Christmas Train Controller

WiFi controller for Christmas train with ESP32, motor relay and audio playback via MAX98357A.

[🇮🇹 Versione Italiana](README.it.md)

## Required Hardware

- **ESP32 DevKit** - Main microcontroller
- **5V 1-channel Relay Module** - Motor control
- **MAX98357A I2S amplifier** - Audio playback
- **8Ω 3W Speaker** - Audio output
- **Power Supply** - 5V for ESP32 (mini USB power bank or battery holder)
- **Dupont cables** - Connections

## Wiring Diagram

### Motor Relay
```
Relay Module   →    ESP32
-----------         -----
VCC            →    5V (VIN)
GND            →    GND
IN             →    GPIO 2

Relay Module   →    Locomotive
-----------         ----------
COM            →    Button wire 1
NO             →    Button wire 2
```

### MAX98357A Audio
```
MAX98357A      →    ESP32
---------           -----
VIN            →    5V (VIN)
GND            →    GND
BCLK           →    GPIO 26
LRC (LRCLK)    →    GPIO 25
DIN            →    GPIO 22

MAX98357A      →    Speaker
---------           -------
Speaker+       →    Speaker+ (red)
Speaker-       →    Speaker- (black)
```

## Configuration

### 1. Configure WiFi
Copy the example configuration file and add your WiFi credentials:

```bash
cp src/config.h.example src/config.h
```

Then edit `src/config.h` and insert your WiFi credentials:

```cpp
const char* WIFI_SSID = "YOUR_WIFI";
const char* WIFI_PASSWORD = "YOUR_PASSWORD";
```

**Note:** The `config.h` file is ignored by git to keep your credentials private.

### 2. Build and Upload
In PlatformIO:
1. Click "Build" (checkmark icon at the bottom)
2. Connect the ESP32 via USB
3. Click "Upload" (right arrow icon at the bottom)

### 3. Open Serial Monitor
Click the serial monitor icon at the bottom to see the ESP32's IP address.

### 4. Open the Web App
In your phone/PC browser (connected to the same WiFi), go to the address shown in the Serial Monitor:
```
http://192.168.x.x
```

## Features

### Web App (already implemented)
- **Motor Control**: Start/Stop buttons
- **Audio Control**: Play/Stop music buttons
- **Volume**: 0-100% slider
- **Status**: real-time status display

### REST API
If you want to control the train from scripts or other apps:

- `GET /motor/start` - Start motor
- `GET /motor/stop` - Stop motor
- `GET /audio/start` - Start audio
- `GET /audio/stop` - Stop audio
- `GET /audio/volume?value=50` - Set volume (0-100)
- `GET /status` - Get current status (JSON)

## Audio Implementation

The current firmware includes a basic audio control interface. To implement full audio playback:

1. Connect the MAX98357A hardware as shown in the wiring diagram
2. Add audio files to the ESP32 flash memory (SPIFFS or LittleFS)
3. Implement I2S playback in the `audioStart()` and `audioStop()` functions in `main.cpp`
4. Use libraries like ESP32-audioI2S for audio file decoding and playback

### Pin Configuration (optional)
If you want to change the pins, edit the `src/config.h` file:

```cpp
#define RELAY_PIN 2      // Motor relay pin
#define I2S_BCLK 26      // I2S pins
#define I2S_LRC  25
#define I2S_DOUT 22
```

## Troubleshooting

### ESP32 won't connect to WiFi
- Verify credentials in `config.h`
- Make sure WiFi is 2.4GHz (ESP32 doesn't support 5GHz)
- Check the Serial Monitor for error messages

### Relay doesn't switch
- Verify connections (VCC, GND, GPIO 2)
- Some relay modules need LOW signal to activate (invert HIGH/LOW in the code)

### Compilation errors
- Wait for PlatformIO to download all libraries
- If it persists, try: PlatformIO → Clean → Build

## Notes

- The ESP32 must always be powered on to receive WiFi commands
- The locomotive's physical button only powers the motor (direct power)
- The ESP32-controlled relay acts as a switch between the button and the motor
- Locomotive voltage: 6V (4 AA batteries)
- ESP32 power supply: separate (5V USB)
