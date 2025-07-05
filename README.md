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
  - **Christmas** (red-warm-white-green)  
  - **Irish** (green-white-orange)  
- **Adjustable scroll speed** (1–10)  
- **Web UI** for message/effect/color editing  
- **On/off toggle** & **brightness control**  
- **Factory reset** via web or **physical button** (5 sec hold)  
- **Optional HTTP basic auth**  

---

## 🔧 Hardware

- ESP32 development board  
- 8x32 Flecible LED Matrix Panels (3 panels is optimal)  
- Push-button wired to GPIO 15 (factory-reset & reboot)  
- 5V Power Supply

---

## 📦 Software Dependencies

Install via Library Manager or manually:

- [Adafruit NeoMatrix](https://github.com/adafruit/Adafruit_NeoMatrix)  
- [Adafruit NeoPixel](https://github.com/adafruit/Adafruit_NeoPixel)  
- [Adafruit GFX](https://github.com/adafruit/Adafruit-GFX-Library)  
- [WiFiManager](https://github.com/tzapu/WiFiManager)  
- [ESPAsyncWebServer](https://github.com/ESP32Async/ESPAsyncWebServer) *(if you swap WebServer for AsyncWebServer)* (NOT TESTED!!! Try at own risk)
- EEPROM (built-in)  
- math.h (built-in)  

---

## ⚙️ Installation & Setup

1. Download ZIP
2. Open Arduino Sketch File
3. Install ESP32 dependencies if needed
4. Install NeoMatrix and WifiManager libraries.
5. Connect ESP32 via USB
6. Upload code

## After Installation

1. Connect to LED SIGN wifi network
2. Go to wifi setup and select network to connect sign to
3. Once sign is connected the ip address will start scrolling on the display
4. Go to ip address shown on the display and enjoy your new sign
