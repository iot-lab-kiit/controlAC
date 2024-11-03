# ACCONtROL

This repo contains a project for controlling an air conditioning unit wirelessly using an ESP8266 microcontroller. The system allows users to operate the AC unit via a web interface accessible from a smartphone or PC, eliminating the need for a traditional remote control. This project is part of a broader lab automation initiative aimed at enhancing the convenience and efficiency of managing home appliances.

## Features

- **Wireless Control**: Operate your AC unit from anywhere within Wi-Fi range using a web browser.
- **User-Friendly Interface**: Simple web interface to control power, temperature, fan speed, and mode.
- **Customizable Settings**: Easily modify the code to adapt to different AC models or functionalities.
- **Persistent Settings**: The settings are saved in the SPIFFS file system, allowing them to persist across reboots.

## Requirements

- **Hardware**:
  - ESP8266 microcontroller (e.g., NodeMCU, Wemos D1 R1 Mini)
  - IR LED for sending commands to the AC unit
  - Power supply for the ESP8266

- **Software**:
  - Arduino IDE with ESP8266 board support
  - Required libraries:
    - ESP8266WiFi
    - IRremoteESP8266
    - IRsend
    - FS
    - ArduinoJson

## Schematics

![Schematics](/assets/schematic.png)
![2d layout](/assets/pcb2d.png)
![3d layout](/assets/pcb3d.png)

## Installation

1. **Clone the Repository**:

   ```bash
   git clone https://github.com/iot-lab-kiit/controlAC
   cd controlAC
   ```

2. **Open in Arduino IDE**:

   - Open the `.ino` file in Arduino IDE.

3. **Install Libraries**:

   - Ensure you have the required libraries installed via the Library Manager in Arduino IDE.

4. **Configure Wi-Fi Credentials**:

   - Update the `ssid` and `password` variables in the code with your Wi-Fi credentials.

5. **Upload Code**:
   - Connect your ESP8266 to your computer and upload the code using Arduino IDE.

## Usage

1. After uploading, open the Serial Monitor (set to 115200 baud) to view connection status.
2. Once connected, access the web interface by entering the ESP8266's IP address in your web browser.
3. Use the buttons on the interface to control power, temperature, fan speed, and mode of your AC unit.

## Customization

The code is designed to be easily modified for different AC models or additional features:

- To change IR codes for different AC brands/models, update the corresponding functions (`convertFan`, `convertMode`, etc.) with appropriate values.
- Adjust temperature limits and fan speed settings as needed.

## Credits

This project is built upon various open-source resources. Special thanks to:

- [IRremoteESP8266](https://github.com/crankyoldgit/IRremoteESP8266) for providing IR communication capabilities.
- [ArduinoJson](https://github.com/bblanchon/ArduinoJson) for handling JSON data storage.
- [ESP8266WiFi](https://github.com/esp8266/Arduino) library for Wi-Fi connectivity.

## License

This project is licensed under the MIT License. See the LICENSE file for details.

---
