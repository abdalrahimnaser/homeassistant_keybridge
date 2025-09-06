# KeyBridge: Keyboard to Home Assistant MQTT Bridge
<p align="center">
  <img width="421" height="410" alt="Untitled Diagram drawio" src="https://github.com/user-attachments/assets/68ae9a1e-cb8d-4bdf-9eef-5706d07371ea" />
</p>

This guide will help you set up your KeyBridge (or your esp32 devkit setup as per `/hardware/devkit_setup`) to convert keyboard and macropad commands into Home Assistant MQTT messages for triggering automations.
Note: make sure that your version of esp32 supports USB Host (ESP32S3 tested working).

## Prerequisites

Before getting started, ensure your Home Assistant MQTT configuration is properly set up. For detailed instructions, refer to this tutorial: https://www.youtube.com/watch?v=ImBv2mtaRR0&pp=ygUTaG9tZSBhc3Npc3RhbnQgbXF0dA%3D%3D

## Firmware Setup

### 1. Load the Base Project
In ESP-IDF, load the **HID** example project located under **Host** examples.

### 2. Replace Source Files
Replace all files in the `/main` directory with the files from the `/FW` directory in this repository.

### 3. Configure Network Settings
Run the following command to open the configuration menu:
```bash
idf.py menuconfig
```
Navigate to **Example Connection Configuration** and set:
- Your WiFi network SSID
- Your WiFi password

### 4. Configure MQTT Settings
In `firmware_main.c` (line 56), update the following parameters:
- **MQTT Broker URL**: Replace with your Home Assistant IP and MQTT port (keep the `mqtt://` prefix)
- **MQTT Username**: Set your MQTT broker username
- **MQTT Password**: Set your MQTT broker password

### 5. Build and Flash
Build and flash the project to your ESP32, you can do that in one go via the fire icon in the bottom bar of VS Code (must have the ESP IDF extension set up):

## Home Assistant Automation Setup

### Creating MQTT-Triggered Automations

1. Navigate to **Settings** → **Automations & Scenes** → **Create Automation** → **Create New Automation**

2. **Configure the Trigger ("When" section)**:
   - Search for and select the **MQTT** option
   - Set the **Topic** to: `/keybridge/key`
   - Set the **Payload** to your desired key identifier (e.g., `0` for the zero key)

3. **Configure the Action ("Then Do" section)**:
   - Specify the action to perform when the key is pressed
   - Example: Create a persistent notification for testing purposes

## Example Use Case

A basic automation might trigger a persistent notification whenever the `0` key is pressed on your connected keyboard/macropad, demonstrating successful communication between your KeyBridge and Home Assistant.
<p align="center"> <img width="489" height="212" alt="image" src="https://github.com/user-attachments/assets/82b15e7d-d673-4ae5-a9f8-e6eabc46f880" /> </p>

You can expand this setup to control lights, switches, scenes, or any other Home Assistant entities based on different key combinations from your input device.
