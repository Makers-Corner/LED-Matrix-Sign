# ESP32 LED Matrix Sign

A Wi-Fi-controlled scrolling LED sign built on an ESP32 + NeoMatrix.  
Customize up to 10 messages, pick from a variety of color and animation effects—including holiday themes—and control everything from a mobile browser!

---

## 📋 Features

- **Up to 10 messages** stored in EEPROM  
- **10+ built-in effects**, including:  
  - None (static)  
  - Rainbow  
  - Color Wave  
  - Gradient  
  - Hold Static  
  - Random Colors  
  - Random Effects  
  - **’Murica** (red-white-blue)  
  - **Christmas** (red-white-green)  
  - **Irish** (green-white-orange)  
- **Adjustable scroll speed** (1–10)  
- **Web UI** for message/effect/color editing  
- **On/off toggle** & **brightness control**  
- **Factory reset** via web or **physical button** (5 sec hold)  
- **Optional HTTP basic auth**  

---

## 🔧 Hardware

- ESP32 development board  
- 8x32 Flexible LED Matrix Panels (3 panels is optimal)  
- Push-button wired to GPIO 15 (factory-reset & reboot)  
- 5V Power Supply

---

## 📦 Software Dependencies

Install via Library Manager or manually:

- [Adafruit NeoMatrix](https://github.com/adafruit/Adafruit_NeoMatrix)  
- [Adafruit NeoPixel](https://github.com/adafruit/Adafruit_NeoPixel)  
- [Adafruit GFX](https://github.com/adafruit/Adafruit-GFX-Library)  
- [WiFiManager](https://github.com/tzapu/WiFiManager)  
- [ESPAsyncWebServer](https://github.com/ESP32Async/ESPAsyncWebServer) *(OPTIONAL if you swap WebServer for AsyncWebServer)* (NOT TESTED!!! TRY AT OWN RISK!!!)
- EEPROM (built-in)  
- math.h (built-in)  

---

## ⚙️ Installation & Setup

1. Download ZIP
2. Open Arduino Sketch File
3. Install ESP32 dependencies if needed
4. Install NeoMatrix and WifiManager libraries
5. Modify line 10 "#DEFINE MATRIX WIDTH" to desired length (default is "96" for 3 - 8x32 panels)
6. Connect ESP32 via USB
7. Upload code

## After Installation

1. Connect to LED SIGN wifi network
2. Once connected it should auto launch your web browser to the setup page
3. If it does not automatically launch, open your browser and navigate to 192.168.4.1
4. Once sign is connected your selected network the ip address will start scrolling on the display
5. Go to ip address shown on the display
6. enjoy your new sign


9. ![Screenshot_20250708-194555](https://github.com/user-attachments/assets/1f5e98ce-4765-4c86-a952-2f8a3418e757)

