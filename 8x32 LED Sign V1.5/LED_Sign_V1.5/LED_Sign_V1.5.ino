#include <Adafruit_GFX.h>
#include <Adafruit_NeoMatrix.h>
#include <Adafruit_NeoPixel.h>
#include <WiFiManager.h>
#include <WebServer.h>
#include <EEPROM.h>
#include <math.h>

#define PIN 5
#define MATRIX_WIDTH 96
#define MATRIX_HEIGHT 8
#define EEPROM_SIZE 2048
#define MAX_MESSAGES 10
#define EEPROM_MAGIC 0x42

#define RESET_BUTTON_PIN 15
#define RESET_HOLD_MS     5000   // 5 seconds for factory reset

// Scrolling timing
uint8_t  scrollSpeed    = 1;    // pixels/frame
uint8_t  speedSetting   = 1;    // slider 1–10
uint16_t frameDelay     = 30;   // ms/frame

Adafruit_NeoMatrix matrix = Adafruit_NeoMatrix(
  MATRIX_WIDTH, MATRIX_HEIGHT, PIN,
  NEO_MATRIX_BOTTOM + NEO_MATRIX_RIGHT + NEO_MATRIX_COLUMNS + NEO_MATRIX_ZIGZAG,
  NEO_GRB + NEO_KHZ800
);
WebServer server(80);

String   messages[MAX_MESSAGES];
uint8_t  effects[MAX_MESSAGES];  // 0=None,1=Rainbow,2=Color Wave,3=Gradient,4=Hold Static,5=Random Colors,6=Random Effects,7='Murica,8=Christmas,9=Irish
uint16_t colors[MAX_MESSAGES];

bool powerOn             = true;
bool showIPAddress       = false;
int  brightness          = 40;
int  messageCount        = 0;
int  currentMessageIndex = 0;

bool  authEnabled       = true;
char  www_username[16]  = "admin";
char  www_password[16]  = "ledsign123";

void saveToEEPROM() {
  EEPROM.write(0, EEPROM_MAGIC);
  EEPROM.write(1, messageCount);
  EEPROM.write(2, powerOn ? 1 : 0);
  EEPROM.write(3, brightness);
  EEPROM.write(4, authEnabled ? 1 : 0);
  int addr = 5;
  EEPROM.write(addr++, strlen(www_username));
  for (int i = 0; i < strlen(www_username); i++)
    EEPROM.write(addr++, www_username[i]);
  EEPROM.write(addr++, strlen(www_password));
  for (int i = 0; i < strlen(www_password); i++)
    EEPROM.write(addr++, www_password[i]);
  for (int i = 0; i < messageCount; i++) {
    EEPROM.write(addr++, messages[i].length());
    for (char c : messages[i]) EEPROM.write(addr++, c);
    EEPROM.write(addr++, effects[i]);
    EEPROM.write(addr++, (colors[i] >> 8) & 0xFF);
    EEPROM.write(addr++, colors[i] & 0xFF);
  }
  EEPROM.commit();
}

void loadFromEEPROM() {
  if (EEPROM.read(0) != EEPROM_MAGIC) {
    messageCount = 0; powerOn = true; brightness = 40;
    strcpy(www_username, "admin");
    strcpy(www_password, "ledsign123");
    authEnabled = false;
    return;
  }
  messageCount = EEPROM.read(1);
  powerOn      = EEPROM.read(2);
  brightness   = EEPROM.read(3);
  authEnabled  = EEPROM.read(4);

  int addr = 5;
  int ulen = EEPROM.read(addr++);
  for (int i = 0; i < ulen; i++) www_username[i] = EEPROM.read(addr++);
  www_username[ulen] = '\0';
  int plen = EEPROM.read(addr++);
  for (int i = 0; i < plen; i++) www_password[i] = EEPROM.read(addr++);
  www_password[plen] = '\0';
  messageCount = min(messageCount, MAX_MESSAGES);
  for (int i = 0; i < messageCount; i++) {
    int len = EEPROM.read(addr++);
    char buf[65];
    for (int j = 0; j < len && j < 64; j++) buf[j] = EEPROM.read(addr++);
    buf[len] = '\0';
    messages[i] = String(buf);
    effects[i]  = EEPROM.read(addr++);
    colors[i]   = (EEPROM.read(addr++) << 8) | EEPROM.read(addr++);
  }
}

bool authenticate() {
  if (!authEnabled) return true;
  return server.authenticate(www_username, www_password);
}

void handleRoot() {
  if (!authenticate()) { 
    server.requestAuthentication(); 
    return; 
  }

  String html = R"HTML(
<!DOCTYPE html><html><head><meta name="viewport" content="width=device-width">
<style>
  body{font-family:sans-serif;background:#111;color:#eee;padding:1em;}
  input,select{width:100%;padding:10px;margin:10px 0;border-radius:5px;border:none;}
  button{padding:10px;border:none;border-radius:5px;margin-top:5px;cursor:pointer;}
</style>
<script>
let msgIndex = 0;
function addMessage(t='', e=0, c='#FFFFFF') {
  if (msgIndex >= 10) return;
  let div = document.createElement('div');
  div.id = 'msg' + msgIndex;
  div.innerHTML = `
    <input name='msg${msgIndex}' value='${t}'>
    <select name='eff${msgIndex}'>
      <option value='0'${e==0?' selected':''}>None</option>
      <option value='1'${e==1?' selected':''}>Rainbow</option>
      <option value='2'${e==2?' selected':''}>Color Wave</option>
      <option value='3'${e==3?' selected':''}>Gradient</option>
      <option value='4'${e==4?' selected':''}>Hold Static</option>
      <option value='5'${e==5?' selected':''}>Random Colors</option>
      <option value='6'${e==6?' selected':''}>Random Effects</option>
      <option value='7'${e==7?' selected':''}>'Murica</option>
      <option value='8'${e==8?' selected':''}>Christmas</option>
      <option value='9'${e==9?' selected':''}>Irish</option>
    </select>
    <input type='color' name='col${msgIndex}' value='${c}'>
    <button type='button' style='background:#f44336;color:#fff;margin-left:10px;'
            onclick='removeMsg(${msgIndex})'>Remove</button>
    <hr>
  `;
  document.getElementById('messages').appendChild(div);
  msgIndex++;
  document.getElementById('count').value = msgIndex;
}
function removeMsg(i) {
  let e = document.getElementById('msg' + i);
  if (e) e.remove();
}
function updateCount() {
  let blks = document.querySelectorAll('[id^=msg]');
  document.getElementById('count').value = blks.length;
  blks.forEach((b,i) => {
    b.querySelectorAll('input,select').forEach(inp => {
      inp.name = inp.name.replace(/\d+/, i);
    });
  });
}
</script>
</head><body>
<h2>LED SIGN CONTROL</h2>
<form action="/set" onsubmit="updateCount()">
  <input type="hidden" id="count" name="count" value="0">
  <div id="messages"></div>
  <button type="button" style="background:#2196F3;color:#fff;" onclick="addMessage()">Add Message</button>
  <button type="submit" style="background:#4CAF50;color:#fff;margin-left:5px;">Save</button>
</form>
<form action="/toggle">
  <button style="background:#FF5722;color:#fff;">Power: )HTML"
    + String(powerOn ? "ON" : "OFF") + R"HTML(</button>
</form>
<form action="/settings">
  <button style="background:#03A9F4;color:#fff;">Settings</button>
</form>
)HTML";

  // pre-populate saved messages
  for (int i = 0; i < messageCount; i++) {
    char hex[8];
    uint16_t c = colors[i];
    uint8_t r = (c>>11)*255/31,
            g = ((c>>5)&0x3F)*255/63,
            b = (c&0x1F)*255/31;
    sprintf(hex, "#%02X%02X%02X", r, g, b);
    html += "<script>addMessage(`" 
          + messages[i] 
          + "`, " 
          + String(effects[i]) 
          + ", `" 
          + String(hex) 
          + "`);</script>\n";
  }

  html += R"HTML(
<form action="/setspeed" method="POST" style="margin-top:1em;">
  <label>Scroll Speed: <span id="speedVal">)HTML"
    + String(speedSetting) + R"HTML(</span></label>
  <input type="range" name="speed" min="1" max="10" value=")HTML"
    + String(speedSetting) + R"HTML(" 
         oninput="document.getElementById('speedVal').innerText=this.value;"/>
  <button style="background:#FF9800;color:#000;">Set Speed</button>
</form>
</body></html>
)HTML";

  server.send(200, "text/html", html);
}

void handleSetText() {
  if (!authenticate()) { server.requestAuthentication(); return; }
  int cnt = server.arg("count").toInt(), v = 0;
  for (int i = 0; i < cnt && v < MAX_MESSAGES; i++) {
    String k = "msg" + String(i);
    if (!server.hasArg(k) || server.arg(k).isEmpty()) continue;
    messages[v] = server.arg(k);
    effects[v]  = server.arg("eff" + String(i)).toInt();
    String h    = server.arg("col" + String(i));
    int rr = strtol(h.substring(1,3).c_str(), NULL, 16),
        gg = strtol(h.substring(3,5).c_str(), NULL, 16),
        bb = strtol(h.substring(5,7).c_str(), NULL, 16);
    colors[v++] = matrix.Color(rr, gg, bb);
  }
  messageCount = v;
  showIPAddress = false;
  saveToEEPROM();
  server.sendHeader("Location", "/");
  server.send(303);
}

void handleToggle() {
  if (!authenticate()) { server.requestAuthentication(); return; }
  powerOn = !powerOn;
  saveToEEPROM();
  server.sendHeader("Location", "/");
  server.send(303);
}

void handleSettings() {
  if (!authenticate()) { server.requestAuthentication(); return; }
  int pct = (brightness * 100 + 127)/255;
  String html = R"HTML(
<!DOCTYPE html><html><head><meta name="viewport" content="width=device-width">
<style>body{font-family:sans-serif;background:#111;color:#eee;padding:1em;}input,button{width:100%;padding:10px;margin:10px 0;border-radius:5px;border:none;}button{background:#2196F3;color:#fff;cursor:pointer;}</style>
</head><body>
<h2>Settings</h2>
<form action="/setcreds" method="POST">
  <input name="user" value=")HTML" + String(www_username) + R"HTML(" placeholder="Username">
  <input name="pass" value=")HTML" + String(www_password) + R"HTML(" placeholder="Password">
  <button>Save Login</button>
</form>
<form action="/setauth" method="POST">
  <label><input type="checkbox" name="auth" )HTML" + (authEnabled?"checked":"") + R"HTML(> Enable login</label>
  <button>Apply Auth</button>
</form>
<form action="/apply" method="POST" style="margin-top:1em;">
  <label>Brightness: <span id="brightVal">)HTML" + String(pct) + R"HTML(%</span></label>
  <input type="range" name="brightness" min="1" max="255" value=")HTML" + String(brightness) + R"HTML(" oninput="document.getElementById('brightVal').innerText=Math.round(this.value/255*100)+'%';"/>
  <button>Apply Brightness</button>
</form>
<form action="/reset" onsubmit="return confirm('Factory reset? This erases WiFi & msgs.');">
  <button style="background:#f44336;color:#fff;">Factory Reset</button>
</form>
<form action="/"><button style="background:#757575;color:#fff;">Back</button></form>
</body></html>
)HTML";
  server.send(200, "text/html", html);
}

void handleSetCreds() {
  if (!authenticate()) { server.requestAuthentication(); return; }
  String u = server.arg("user"), p = server.arg("pass");
  if (u.length() > 0 && u.length() <= 15 && p.length() > 0 && p.length() <= 15) {
    u.toCharArray(www_username, 16);
    p.toCharArray(www_password, 16);
    saveToEEPROM();
    server.send(200, "text/plain", "Saved. Rebooting...");
    delay(500);
    ESP.restart();
  } else {
    server.send(400, "text/plain", "Invalid creds");
  }
}

void handleSetAuth() {
  if (!authenticate()) { server.requestAuthentication(); return; }
  authEnabled = server.hasArg("auth");
  saveToEEPROM();
  server.sendHeader("Location", "/settings");
  server.send(303);
}

void handleApplySettings() {
  if (!authenticate()) { server.requestAuthentication(); return; }
  brightness = constrain(server.arg("brightness").toInt(), 1, 255);
  matrix.setBrightness(brightness);
  saveToEEPROM();
  server.sendHeader("Location", "/settings");
  server.send(303);
}

void handleReset() {
  if (!authenticate()) { server.requestAuthentication(); return; }
  for (int i = 0; i < EEPROM_SIZE; i++) EEPROM.write(i, 0);
  EEPROM.commit();
  WiFiManager wm; wm.resetSettings();
  server.send(200, "text/plain", "Factory reset. Rebooting...");
  delay(1000);
  ESP.restart();
}

void handleSetSpeed() {
  if (!authenticate()) { server.requestAuthentication(); return; }
  speedSetting = constrain(server.arg("speed").toInt(), 1, 10);
  float n = (speedSetting - 1) / 9.0f, c = pow(n, 0.1453f);
  frameDelay = round(200 - c * (200 - 30));
  server.sendHeader("Location", "/");
  server.send(303);
}

void setup() {
  Serial.begin(115200); delay(1000);
  pinMode(RESET_BUTTON_PIN, INPUT_PULLUP);
  EEPROM.begin(EEPROM_SIZE);
  matrix.begin(); matrix.setTextWrap(false);
  randomSeed(esp_random());
  loadFromEEPROM();
  matrix.setBrightness(brightness);
  if (messageCount == 0) showIPAddress = true;
  WiFiManager wm;
  if (!wm.autoConnect("LED_Sign_Setup")) { delay(3000); ESP.restart(); }
  server.on("/",        handleRoot);
  server.on("/set",     handleSetText);
  server.on("/toggle",  handleToggle);
  server.on("/settings",handleSettings);
  server.on("/setcreds",HTTP_POST,handleSetCreds);
  server.on("/setauth", HTTP_POST,handleSetAuth);
  server.on("/apply",   HTTP_POST,handleApplySettings);
  server.on("/reset",   handleReset);
  server.on("/setspeed",HTTP_POST,handleSetSpeed);
  server.begin();
}

void loop() {
  // --- Reset / Reboot Button Logic ---
  static unsigned long resetPressStart = 0;
  static bool longPressTriggered = false;

  if (digitalRead(RESET_BUTTON_PIN) == LOW) {
    if (resetPressStart == 0) {
      resetPressStart = millis();
      longPressTriggered = false;
    }
    else if (!longPressTriggered && millis() - resetPressStart >= RESET_HOLD_MS) {
      longPressTriggered = true;
      for (int i = 0; i < EEPROM_SIZE; i++) EEPROM.write(i, 0);
      EEPROM.commit();
      WiFiManager wm; wm.resetSettings();
      ESP.restart();
    }
  } else {
    if (resetPressStart != 0 && !longPressTriggered) {
      ESP.restart();
    }
    resetPressStart = 0;
  }

  server.handleClient();

  if (!powerOn) {
    matrix.fillScreen(0);
    matrix.show();
    delay(frameDelay);
    return;
  }

  if (showIPAddress) {
    static String ip = "";
    static int x = MATRIX_WIDTH;
    if (ip.isEmpty()) ip = WiFi.localIP().toString();
    matrix.fillScreen(0);
    matrix.setCursor(x, 0);
    matrix.setTextColor(matrix.Color(255,255,255));
    matrix.print(ip);
    matrix.show();
    x -= scrollSpeed;
    if (x < -(int)ip.length() * 6) x = MATRIX_WIDTH;
    delay(frameDelay);
    return;
  }

  if (messageCount == 0) {
    delay(frameDelay);
    return;
  }

  // Scrolling & effects
  static uint32_t start = 0;
  static bool inProg = true, held = false;
  static int xpos = MATRIX_WIDTH;
  static uint16_t letterCols[64];
  static uint8_t cycleEff = 0;

  String msg = messages[currentMessageIndex];
  uint8_t cfgEff = effects[currentMessageIndex];
  int totalW = msg.length() * 6;

  // Hold-static effect
  if (cfgEff == 4) {
    if (!held) {
      matrix.fillScreen(0);
      matrix.setCursor((matrix.width() - totalW) / 2, 0);
      matrix.setTextColor(colors[currentMessageIndex]);
      matrix.print(msg);
      matrix.show();
      start = millis();
      held = true;
    }
    if (millis() - start >= 2000) {
      held = false;
      inProg = true;
      currentMessageIndex = (currentMessageIndex + 1) % messageCount;
      xpos = MATRIX_WIDTH;
    }
    delay(frameDelay);
    return;
  }

  // New scroll init
  if (inProg) {
    inProg = false;
    xpos = MATRIX_WIDTH;
    start = millis();
    held = false;
    // random-effects picker (ignores None=0,Hold Static=4)
    if (cfgEff == 6) {
      const uint8_t opts[] = { 1, 2, 3, 5 };
      cycleEff = opts[random(0, sizeof(opts)/sizeof(opts[0]))];
    } else {
      cycleEff = cfgEff;
    }
    // Prepare gradient or random colors
    if (cycleEff == 3) {
      uint16_t base = (start / 20) & 0xFFFF, step = 65536 / msg.length();
      for (int i = 0; i < msg.length() && i < 64; i++) {
        uint16_t h = (base + i * step) & 0xFFFF;
        uint32_t raw = matrix.ColorHSV(h,255,255);
        uint8_t r=(raw>>8)&0xFF, g=(raw>>16)&0xFF, b=raw&0xFF;
        uint16_t s=r+g+b; if (s) { float f=255.0f/s; r=min(int(r*f),255); g=min(int(g*f),255); b=min(int(b*f),255); }
        letterCols[i] = matrix.Color(r,g,b);
      }
    } else if (cycleEff == 5) {
      randomSeed(start);
      for (int i = 0; i < msg.length() && i < 64; i++) {
        uint16_t h = random(0,65536);
        uint32_t raw = matrix.ColorHSV(h,255,255);
        uint8_t r=(raw>>8)&0xFF, g=(raw>>16)&0xFF, b=raw&0xFF;
        uint16_t s=r+g+b; if (s) { float f=255.0f/s; r=min(int(r*f),255); g=min(int(g*f),255); b=min(int(b*f),255); }
        letterCols[i] = matrix.Color(r,g,b);
      }
    }
  }
  if (millis() - start < 2000) {
    delay(frameDelay);
    return;
  }

  // Draw frame
  matrix.fillScreen(0);
  matrix.setCursor(xpos, 0);
  if (cycleEff == 1 || cycleEff == 2) {
    uint32_t now = millis();
    for (int i = 0; i < msg.length(); i++) {
      uint16_t hue = (cycleEff == 1
                       ? (xpos + i * 10) % 256
                       : (uint16_t)(127.5 * (sin((now/400.0) + i*0.3) + 1))
                     );
      matrix.setTextColor(matrix.ColorHSV(hue * 256));
      matrix.print(msg[i]);
    }

  } else if (cycleEff == 3 || cycleEff == 5) {
    for (int i = 0; i < msg.length(); i++) {
      matrix.setTextColor(letterCols[i]);
      matrix.print(msg[i]);
    }

  } else if (cycleEff == 7) {
    // 'Murica: Red, White, Blue
    for (int i = 0; i < msg.length(); i++) {
      uint16_t col;
      switch (i % 3) {
        case 0: col = matrix.Color(255, 0,   0  ); break; // red
        case 1: col = matrix.Color(255, 255, 255); break; // white
        default:col = matrix.Color(0,   0,   255); break; // blue
      }
      matrix.setTextColor(col);
      matrix.print(msg[i]);
    }

  } else if (cycleEff == 8) {
    // Christmas: Red, Warm-White, Green
    for (int i = 0; i < msg.length(); i++) {
      uint16_t col;
      switch (i % 3) {
        case 0: col = matrix.Color(255, 0,   0  ); break; // red
        case 1: col = matrix.Color(255, 244, 229); break; // warm white
        default:col = matrix.Color(0,   255, 0  ); break; // green
      }
      matrix.setTextColor(col);
      matrix.print(msg[i]);
    }

  } else if (cycleEff == 9) {
    // Irish: Green, White, Orange
    for (int i = 0; i < msg.length(); i++) {
      uint16_t col;
      switch (i % 3) {
        case 0: col = matrix.Color(0,   255, 0  ); break; // green
        case 1: col = matrix.Color(255, 255, 255); break; // white
        default:col = matrix.Color(255, 165, 0  ); break; // orange
      }
      matrix.setTextColor(col);
      matrix.print(msg[i]);
    }

  } else {
    // Single-color effects (None, Hold Static, or any other)
    matrix.setTextColor(colors[currentMessageIndex]);
    matrix.print(msg);
  }

  matrix.show();
  xpos -= scrollSpeed;
  if (xpos < -totalW) {
    inProg = true;
    currentMessageIndex = (currentMessageIndex + 1) % messageCount;
  }

  delay(frameDelay);
}
