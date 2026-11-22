#include <WiFi.h>
#include <WebServer.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "cyrillic5x7.h"

#define SDA_PIN 21
#define SCL_PIN 22

Adafruit_SSD1306 display(128, 64, &Wire, -1);
WebServer server(80);

const char HTML[] PROGMEM = R"(
<!DOCTYPE html><html><head><meta charset="utf-8">
<meta name="viewport" content="width=device-width,initial-scale=1">
<style>
  body{font-family:sans-serif;background:#121212;color:#fff;padding:20px;max-width:400px;margin:auto}
  input,button{width:100%;box-sizing:border-box;font-size:18px;padding:12px;border-radius:8px;border:none;margin-top:10px}
  input{background:#222;color:#fff;border:1px solid #444}
  button{background:#2e7d32;color:#fff;font-weight:bold}
</style></head>
<body>
  <h3>Текст на SSD1306</h3>
  <form action="/">
    <input name="t" placeholder="Введіть текст (порожньо — очистити)..." autofocus>
    <button type="submit">Вивести на екран</button>
  </form>
</body></html>)";

void showText(String s) {
  display.clearDisplay();
  String out = "";
  int col = 0;

  for (int i = 0; i < s.length(); i++) {
    if (s[i] == '\n') {
      out += '\n';
      col = 0;
      continue;
    }
    if (col++ >= 21) {
      out += '\n';
      col = 1;
    }
    out += s[i];
    if ((uint8_t)s[i] >= 0xD0 && i + 1 < s.length()) {
      out += s[++i];
    }
  }

  printCyrillic(display, out, 0, 0, 1);
  display.display();
}

void handleRoot() {
  if (server.hasArg("t")) {
    showText(server.arg("t"));
  }
  server.send(200, "text/html; charset=utf-8", HTML);
}

void setup() {
  Serial.begin(115200);
  Wire.begin(SDA_PIN, SCL_PIN);

  // Автовизначення адреси 0x3C або 0x3D
  uint8_t addr = 0x3C;
  for (uint8_t a = 1; a < 127; a++) {
    Wire.beginTransmission(a);
    if (Wire.endTransmission() == 0 && (a == 0x3C || a == 0x3D)) {
      addr = a;
      break;
    }
  }

  display.begin(SSD1306_SWITCHCAPVCC, addr);
  display.setTextColor(SSD1306_WHITE);

  WiFi.softAP("ESP32-OLED");
  server.on("/", handleRoot);
  server.begin();

  String info = "Wi-Fi: ESP32-OLED\nIP: " + WiFi.softAPIP().toString();
  Serial.println(info);
  showText(info);
}

void loop() {
  server.handleClient();
}