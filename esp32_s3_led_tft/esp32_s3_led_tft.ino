/*
  ESP32-S3 LED controller
  - ILI9341 3.2" SPI TFT with XPT2046 touch (LovyanGFX)
  - One on-screen toggle button
  - Same toggle served as a web page over Wi-Fi (screen and browser stay in sync)

  Library needed: LovyanGFX (Library Manager)
  Board: ESP32S3 Dev Module
*/

#define LGFX_USE_V1
#include <LovyanGFX.hpp>
#include <WiFi.h>
#include <WebServer.h>
#include <Preferences.h>

// ---------- USER SETTINGS ----------
const char* WIFI_SSID = "YOUR_WIFI_NAME";
const char* WIFI_PASS = "YOUR_WIFI_PASSWORD";

const int LED_PIN = 4;     // GPIO4 -> 220 ohm -> LED(+), LED(-) -> GND
const int CAL_PIN = 0;    // hold during reset to redo touch calibration
// -----------------------------------

// ---------- Display + touch config ----------
class LGFX : public lgfx::LGFX_Device {
  lgfx::Panel_ILI9341 _panel;
  lgfx::Bus_SPI _bus;
  lgfx::Touch_XPT2046 _touch;

public:
  LGFX() {
    {
      auto cfg = _bus.config();
      cfg.spi_host = SPI2_HOST;
      cfg.spi_mode = 0;
      cfg.freq_write = 40000000;
      cfg.freq_read = 16000000;
      cfg.spi_3wire = false;
      cfg.use_lock = true;
      cfg.dma_channel = SPI_DMA_CH_AUTO;
      cfg.pin_sclk = 12;
      cfg.pin_mosi = 11;
      cfg.pin_miso = 13;
      cfg.pin_dc = 9;
      _bus.config(cfg);
      _panel.setBus(&_bus);
    }
    {
      auto cfg = _panel.config();
      cfg.pin_cs = 10;
      cfg.pin_rst = 14;
      cfg.pin_busy = -1;
      cfg.panel_width = 240;
      cfg.panel_height = 320;
      cfg.offset_x = 0;
      cfg.offset_y = 0;
      cfg.readable = true;
      cfg.invert = false;
      cfg.rgb_order = false;
      cfg.bus_shared = true;   // touch shares the SPI bus
      _panel.config(cfg);
    }
    {
      auto cfg = _touch.config();
      cfg.x_min = 300;
      cfg.x_max = 3900;
      cfg.y_min = 300;
      cfg.y_max = 3900;
      cfg.pin_int = -1;        // T_IRQ not used
      cfg.bus_shared = true;
      cfg.offset_rotation = 0;
      cfg.spi_host = SPI2_HOST;
      cfg.freq = 1000000;
      cfg.pin_sclk = 12;
      cfg.pin_mosi = 11;
      cfg.pin_miso = 13;
      cfg.pin_cs = 8;
      _touch.config(cfg);
      _panel.setTouch(&_touch);
    }
    setPanel(&_panel);
  }
};

LGFX lcd;
WebServer server(80);
Preferences prefs;

bool ledOn = false;
bool wasTouched = false;
unsigned long lastToggle = 0;

// Button geometry (landscape 320x240)
const int BTN_X = 50, BTN_Y = 85, BTN_W = 220, BTN_H = 100;

// ---------- Web page ----------
const char PAGE[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width, initial-scale=1">
<title>ESP32 LED Controller</title>
<style>
body{margin:0;min-height:100vh;display:flex;align-items:center;justify-content:center;background:#f1f5f9;font-family:Arial,sans-serif}
.card{width:420px;max-width:92%;background:#fff;padding:35px;border-radius:18px;box-shadow:0 10px 30px rgba(0,0,0,.12);text-align:center}
h1{color:#1f2937;margin-bottom:8px}.sub{color:#64748b;margin-bottom:24px}
button{border:0;border-radius:10px;padding:18px 40px;color:#fff;font-size:18px;font-weight:bold;cursor:pointer;min-width:220px}
.on{background:#16a34a}.off{background:#6b7280}
</style>
</head>
<body>
<div class="card">
<h1>ESP32 LED Controller</h1>
<div class="sub">Wi-Fi &rarr; ESP32-S3 &rarr; LED</div>
<button id="b" class="off" onclick="t()">...</button>
</div>
<script>
const b=document.getElementById('b');
function show(on){b.textContent=on?'LED ON (click to turn OFF)':'LED OFF (click to turn ON)';b.className=on?'on':'off';}
async function t(){try{const r=await fetch('/toggle',{method:'POST'});show((await r.text())==='1');}catch(e){}}
async function p(){try{const r=await fetch('/state');show((await r.text())==='1');}catch(e){}}
p();setInterval(p,1000);
</script>
</body>
</html>
)rawliteral";

// ---------- Drawing ----------
void drawButton() {
  uint32_t bg = ledOn ? lcd.color565(22, 163, 74) : lcd.color565(107, 114, 128);
  lcd.fillRoundRect(BTN_X, BTN_Y, BTN_W, BTN_H, 16, bg);
  lcd.setTextColor(lcd.color565(255, 255, 255), bg);
  lcd.setTextDatum(middle_center);
  lcd.setTextSize(3);
  lcd.drawString(ledOn ? "LED ON" : "LED OFF", BTN_X + BTN_W / 2, BTN_Y + BTN_H / 2 - 12);
  lcd.setTextSize(2);
  lcd.drawString(ledOn ? "tap to turn OFF" : "tap to turn ON", BTN_X + BTN_W / 2, BTN_Y + BTN_H / 2 + 22);
}

void drawFooter(const String& text) {
  uint32_t black = lcd.color565(0, 0, 0);
  lcd.fillRect(0, 205, 320, 35, black);
  lcd.setTextColor(lcd.color565(148, 163, 184), black);
  lcd.setTextDatum(middle_center);
  lcd.setTextSize(2);
  lcd.drawString(text, 160, 222);
}

void drawUI(const String& footer) {
  uint32_t black = lcd.color565(0, 0, 0);
  lcd.fillScreen(black);
  lcd.setTextColor(lcd.color565(255, 255, 255), black);
  lcd.setTextDatum(middle_center);
  lcd.setTextSize(3);
  lcd.drawString("LED Controller", 160, 35);
  drawButton();
  drawFooter(footer);
}

// ---------- LED ----------
void setLed(bool on) {
  ledOn = on;
  digitalWrite(LED_PIN, ledOn ? HIGH : LOW);
  drawButton();
}

// ---------- Touch calibration ----------
void setupTouch() {
  uint16_t cal[8];
  prefs.begin("touch", false);

  bool force = (digitalRead(CAL_PIN) == LOW);
  bool have = (prefs.getBytes("cal", cal, sizeof(cal)) == sizeof(cal));

  if (have && !force) {
    lcd.setTouchCalibrate(cal);
  } else {
    lcd.fillScreen(lcd.color565(0, 0, 0));
    lcd.setTextColor(lcd.color565(255, 255, 255), lcd.color565(0, 0, 0));
    lcd.setTextDatum(middle_center);
    lcd.setTextSize(2);
    lcd.drawString("Touch the corner arrows", 160, 120);
    delay(1500);
    lcd.calibrateTouch(cal, lcd.color565(255, 255, 255), lcd.color565(0, 0, 0), 15);
    prefs.putBytes("cal", cal, sizeof(cal));
  }
  prefs.end();
}

// ---------- Wi-Fi ----------
String connectWifi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  unsigned long start = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - start < 15000) {
    delay(250);
  }
  if (WiFi.status() == WL_CONNECTED) {
    return "http://" + WiFi.localIP().toString();
  }
  return "Wi-Fi not connected";
}

void setup() {
  pinMode(CAL_PIN, INPUT_PULLUP);
  digitalWrite(LED_PIN, LOW);
  pinMode(BOOT_PIN, INPUT_PULLUP);

  lcd.init();
  lcd.setRotation(1);   // landscape, 320x240

  setupTouch();

  String footer = connectWifi();
  drawUI(footer);

  if (WiFi.status() == WL_CONNECTED) {
    server.on("/", HTTP_GET, []() { server.send_P(200, "text/html", PAGE); });
    server.on("/state", HTTP_GET, []() { server.send(200, "text/plain", ledOn ? "1" : "0"); });
    server.on("/toggle", HTTP_POST, []() {
      setLed(!ledOn);
      server.send(200, "text/plain", ledOn ? "1" : "0");
    });
    server.begin();
  }
}

void loop() {
  server.handleClient();

  int32_t x, y;
  bool touched = lcd.getTouch(&x, &y);

  if (touched && !wasTouched && millis() - lastToggle > 250) {
    if (x >= BTN_X && x <= BTN_X + BTN_W && y >= BTN_Y && y <= BTN_Y + BTN_H) {
      setLed(!ledOn);
      lastToggle = millis();
    }
  }
  wasTouched = touched;
}
