# Automatic Pill Dispenser

This project is an automatic pill dispenser using an Arduino. It dispenses pills based on a predefined schedule.

### Hardware Setup

1. **Connect the BME280 sensor:**
   - VCC to 3.3V
   - GND to GND
   - SCL to A5 (or SCL on the Arduino)
   - SDA to A4 (or SDA on the Arduino)

2. **Connect the DHT22 sensor:**
   - VCC to 5V
   - GND to GND
   - Data pin to digital pin 2 (D2)

3. **Connect the SSD1306 OLED screen:**
   - VCC to 3.3V
   - GND to GND
   - SCL to A5 (or SCL on the Arduino)
   - SDA to A4 (or SDA on the Arduino)

4. **Set up the pill dispensing mechanism:**
   - Connect a servo motor to control the pill dispenser.
   - Connect the servo signal pin to a digital pin on the Arduino (e.g., D3).
