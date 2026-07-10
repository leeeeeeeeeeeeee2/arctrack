// ArcTrack — ESP32-C3 model rocket flight computer
//
// - BMP280 barometer  -> altitude (relative to ground) + apogee detection
// - MPU-6050 IMU      -> accel/gyro, launch/burnout detection
// - ATGM336H GPS      -> lat/lon/alt/speed/sats (UART)
// - microSD           -> CSV flight log (SPI)
// - WiFi AP + web page -> live tracking from your phone
// - Buzzer (GPIO0) + servo (GPIO1) for apogee / nose deploy
//
// Pin map MUST match hardware/DESIGN.md / WIRING.md.

#include <Arduino.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <WiFi.h>
#include <WebServer.h>
#include <Adafruit_BMP280.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <TinyGPSPlus.h>
#include <math.h>

// ---------------------------------------------------------------- pins
#define PIN_SDA      4
#define PIN_SCL      5
#define PIN_SD_SCK   6
#define PIN_SD_MOSI  7
#define PIN_SD_MISO  3
#define PIN_SD_CS    10
#define PIN_GPS_RX   20   // ESP receives GPS TX here
#define PIN_GPS_TX   21   // ESP transmits to GPS RX
#define PIN_BUZZER   0    // active buzzer (low side)
#define PIN_SERVO    1    // parachute / nose deploy PWM
// Power LED is hardwired to +3V3 on the PCB (no GPIO)

// ---------------------------------------------------------------- config
static const char*    AP_SSID     = "ArcTrack";
static const char*    AP_PASS     = "launch123";   // >=8 chars
static const uint32_t GPS_BAUD    = 9600;          // ATGM336H default
static const uint32_t SAMPLE_HZ   = 20;            // log/telemetry rate
static const float    SEA_LEVEL_HPA_DEFAULT = 1013.25f;

// launch/apogee detection thresholds
static const float LAUNCH_ACCEL_G = 2.0f;   // > 2g => boost
static const float LAUNCH_ALT_M   = 3.0f;   // > 3m above ground => armed launch

// ---------------------------------------------------------------- globals
Adafruit_BMP280 bmp;              // I2C
Adafruit_MPU6050 mpu;            // I2C
TinyGPSPlus gps;
SPIClass sdSPI(FSPI);
WebServer server(80);

bool haveBMP = false;
bool haveMPU = false;
bool haveSD  = false;
File logFile;

float groundPressureHpa = SEA_LEVEL_HPA_DEFAULT;
float groundAltM        = 0.0f;
bool  launched          = false;
bool  deployed          = false;
uint32_t launchMs       = 0;

// latest telemetry (shared with web handlers)
struct Telemetry {
  float altM = 0;         // relative to ground
  float maxAltM = 0;      // apogee
  float tempC = 0;
  float ax = 0, ay = 0, az = 0;   // g
  float gx = 0, gy = 0, gz = 0;   // deg/s
  double lat = 0, lon = 0;
  float gpsAltM = 0;
  float speedKmh = 0;
  uint32_t sats = 0;
  bool gpsFix = false;
  uint32_t tMs = 0;
} tel;

// ---------------------------------------------------------------- helpers
// Active buzzer: drive low side (pin to GND side of buzzer)
static void buzzOn()  { digitalWrite(PIN_BUZZER, LOW); }
static void buzzOff() { digitalWrite(PIN_BUZZER, HIGH); }

static void beep(int n, int ms) {
  for (int i = 0; i < n; i++) { buzzOn(); delay(ms); buzzOff(); delay(ms); }
}

// Simple servo PWM on LEDC (ESP32-C3)
static const int SERVO_CH = 0;
static void servoWriteUs(int us) {
  // 50 Hz, 16-bit duty approx: duty = us / 20000 * 65535
  uint32_t duty = (uint32_t)((us / 20000.0f) * 65535.0f);
  ledcWrite(SERVO_CH, duty);
}
static void servoStow()    { servoWriteUs(1000); }  // ~0 deg — tune for your linkage
static void servoDeploy()  { servoWriteUs(2000); }  // ~180 deg — tune for nose release


// ---------------------------------------------------------------- web page
static const char INDEX_HTML[] PROGMEM = R"HTML(
<!doctype html><html><head><meta charset="utf-8">
<meta name="viewport" content="width=device-width,initial-scale=1">
<title>ArcTrack Live</title>
<style>
  body{font-family:system-ui,Arial,sans-serif;margin:0;background:#0b1020;color:#e7ecf5}
  header{padding:16px;background:#131a33;font-size:20px;font-weight:700}
  .grid{display:grid;grid-template-columns:1fr 1fr;gap:12px;padding:16px}
  .card{background:#151d3b;border-radius:14px;padding:16px}
  .k{font-size:12px;color:#8ea0c8;text-transform:uppercase;letter-spacing:.05em}
  .v{font-size:28px;font-weight:700;margin-top:4px}
  .full{grid-column:1/3}
  a.btn{display:inline-block;margin-top:10px;background:#3b6cff;color:#fff;
        padding:10px 14px;border-radius:10px;text-decoration:none;font-weight:600}
  .dot{height:10px;width:10px;border-radius:50%;display:inline-block;margin-right:6px}
  .ok{background:#2ecc71}.no{background:#e74c3c}
</style></head><body>
<header>&#128640; ArcTrack Live Tracking</header>
<div class="grid">
  <div class="card"><div class="k">Altitude (AGL)</div><div class="v" id="alt">--</div></div>
  <div class="card"><div class="k">Apogee</div><div class="v" id="max">--</div></div>
  <div class="card"><div class="k">Speed (GPS)</div><div class="v" id="spd">--</div></div>
  <div class="card"><div class="k">Temp</div><div class="v" id="tmp">--</div></div>
  <div class="card full"><div class="k">GPS <span id="fixdot" class="dot no"></span><span id="fix">no fix</span> &middot; sats <span id="sat">0</span></div>
    <div class="v" id="pos">--, --</div>
    <a class="btn" id="map" href="#" target="_blank">Open in Maps</a></div>
  <div class="card"><div class="k">Accel |a|</div><div class="v" id="acc">--</div></div>
  <div class="card"><div class="k">Status</div><div class="v" id="st">--</div></div>
</div>
<script>
async function tick(){
  try{
    const r = await fetch('/data'); const d = await r.json();
    alt.textContent = d.alt.toFixed(1)+' m';
    max.textContent = d.max.toFixed(1)+' m';
    spd.textContent = d.spd.toFixed(1)+' km/h';
    tmp.textContent = d.tmp.toFixed(1)+' \u00b0C';
    sat.textContent = d.sats;
    const amag = Math.sqrt(d.ax*d.ax+d.ay*d.ay+d.az*d.az);
    acc.textContent = amag.toFixed(2)+' g';
    st.textContent = d.launched ? 'IN FLIGHT' : 'armed';
    if(d.fix){
      fix.textContent='fix'; fixdot.className='dot ok';
      pos.textContent = d.lat.toFixed(6)+', '+d.lon.toFixed(6);
      map.href = 'https://maps.google.com/?q='+d.lat+','+d.lon;
    } else { fix.textContent='no fix'; fixdot.className='dot no'; }
  }catch(e){}
}
setInterval(tick, 500); tick();
</script></body></html>
)HTML";

static void handleRoot() { server.send_P(200, "text/html", INDEX_HTML); }

static void handleData() {
  char buf[420];
  snprintf(buf, sizeof(buf),
    "{\"t\":%lu,\"alt\":%.2f,\"max\":%.2f,\"tmp\":%.2f,"
    "\"ax\":%.3f,\"ay\":%.3f,\"az\":%.3f,"
    "\"lat\":%.6f,\"lon\":%.6f,\"gpsAlt\":%.1f,\"spd\":%.1f,"
    "\"sats\":%lu,\"fix\":%s,\"launched\":%s}",
    (unsigned long)tel.tMs, tel.altM, tel.maxAltM, tel.tempC,
    tel.ax, tel.ay, tel.az,
    tel.lat, tel.lon, tel.gpsAltM, tel.speedKmh,
    (unsigned long)tel.sats, tel.gpsFix ? "true" : "false",
    launched ? "true" : "false");
  server.send(200, "application/json", buf);
}

// ---------------------------------------------------------------- setup
static void initSensors() {
  Wire.begin(PIN_SDA, PIN_SCL);
  Wire.setClock(400000);

  if (bmp.begin(0x76) || bmp.begin(0x77)) {
    haveBMP = true;
    bmp.setSampling(Adafruit_BMP280::MODE_NORMAL,
                    Adafruit_BMP280::SAMPLING_X2,
                    Adafruit_BMP280::SAMPLING_X16,
                    Adafruit_BMP280::FILTER_X4,
                    Adafruit_BMP280::STANDBY_MS_1);
    Serial.println("BMP280 ok");
  } else Serial.println("BMP280 MISSING");

  if (mpu.begin(0x68)) {
    haveMPU = true;
    mpu.setAccelerometerRange(MPU6050_RANGE_16_G);
    mpu.setGyroRange(MPU6050_RANGE_2000_DEG);
    mpu.setFilterBandwidth(MPU6050_BAND_44_HZ);
    Serial.println("MPU6050 ok");
  } else Serial.println("MPU6050 MISSING");
}

static void calibrateGround() {
  if (!haveBMP) return;
  // average a second of readings to fix the ground reference
  double sum = 0; int n = 0;
  for (int i = 0; i < 50; i++) { sum += bmp.readPressure() / 100.0; n++; delay(20); }
  groundPressureHpa = (float)(sum / n);
  groundAltM = bmp.readAltitude(groundPressureHpa); // ~0
  Serial.printf("Ground ref: %.2f hPa\n", groundPressureHpa);
}

static void initSD() {
  sdSPI.begin(PIN_SD_SCK, PIN_SD_MISO, PIN_SD_MOSI, PIN_SD_CS);
  if (!SD.begin(PIN_SD_CS, sdSPI)) { Serial.println("SD MISSING"); return; }
  haveSD = true;

  // pick a new filename
  char name[20];
  for (int i = 0; i < 1000; i++) {
    snprintf(name, sizeof(name), "/flight%03d.csv", i);
    if (!SD.exists(name)) break;
  }
  logFile = SD.open(name, FILE_WRITE);
  if (logFile) {
    logFile.println("t_ms,alt_m,temp_c,ax,ay,az,gx,gy,gz,lat,lon,gps_alt,spd_kmh,sats,fix,launched");
    logFile.flush();
    Serial.printf("Logging to %s\n", name);
  } else { haveSD = false; Serial.println("SD open FAIL"); }
}

void setup() {
  pinMode(PIN_BUZZER, OUTPUT);
  buzzOff();
  ledcSetup(SERVO_CH, 50, 16);          // 50 Hz servo
  ledcAttachPin(PIN_SERVO, SERVO_CH);
  servoStow();

  Serial.begin(115200);
  delay(300);
  Serial.println("\nArcTrack boot");

  initSensors();
  calibrateGround();

  Serial1.begin(GPS_BAUD, SERIAL_8N1, PIN_GPS_RX, PIN_GPS_TX);

  initSD();

  WiFi.mode(WIFI_AP);
  WiFi.softAP(AP_SSID, AP_PASS);
  Serial.print("AP up: "); Serial.println(WiFi.softAPIP());
  server.on("/", handleRoot);
  server.on("/data", handleData);
  server.begin();

  beep(3, 120);
  Serial.println("ready");
}

// ---------------------------------------------------------------- loop
static void readGPS() {
  while (Serial1.available()) gps.encode(Serial1.read());
  if (gps.location.isValid()) {
    tel.lat = gps.location.lat();
    tel.lon = gps.location.lng();
    tel.gpsFix = true;
  }
  if (gps.altitude.isValid()) tel.gpsAltM = gps.altitude.meters();
  if (gps.speed.isValid())    tel.speedKmh = gps.speed.kmph();
  if (gps.satellites.isValid()) tel.sats = gps.satellites.value();
}

static void sampleSensors() {
  if (haveBMP) {
    tel.altM  = bmp.readAltitude(groundPressureHpa) - groundAltM;
    tel.tempC = bmp.readTemperature();
    if (tel.altM > tel.maxAltM) tel.maxAltM = tel.altM;
  }
  if (haveMPU) {
    sensors_event_t a, g, t;
    mpu.getEvent(&a, &g, &t);
    tel.ax = a.acceleration.x / 9.80665f;
    tel.ay = a.acceleration.y / 9.80665f;
    tel.az = a.acceleration.z / 9.80665f;
    tel.gx = g.gyro.x * 57.29578f;
    tel.gy = g.gyro.y * 57.29578f;
    tel.gz = g.gyro.z * 57.29578f;
  }
  tel.tMs = millis();
}

static void detectLaunch() {
  if (launched) return;
  float amag = sqrtf(tel.ax*tel.ax + tel.ay*tel.ay + tel.az*tel.az);
  if (amag > LAUNCH_ACCEL_G || tel.altM > LAUNCH_ALT_M) {
    launched = true;
    launchMs = millis();
    Serial.println(">>> LAUNCH detected");
    beep(1, 80);
  }
}

// Deploy once after apogee: altitude falls ~3 m from peak, and we've been flying a bit
static void detectApogeeDeploy() {
  if (!launched || deployed) return;
  if (millis() - launchMs < 800) return;          // ignore early noise
  if (tel.maxAltM < 5.0f) return;                 // need some altitude
  if (tel.altM < tel.maxAltM - 3.0f) {
    deployed = true;
    servoDeploy();
    beep(3, 100);
    Serial.println(">>> APOGEE deploy");
  }
}

static void logRow() {
  if (!haveSD || !logFile) return;
  logFile.printf("%lu,%.2f,%.2f,%.3f,%.3f,%.3f,%.2f,%.2f,%.2f,%.6f,%.6f,%.1f,%.1f,%lu,%d,%d\n",
    (unsigned long)tel.tMs, tel.altM, tel.tempC,
    tel.ax, tel.ay, tel.az, tel.gx, tel.gy, tel.gz,
    tel.lat, tel.lon, tel.gpsAltM, tel.speedKmh,
    (unsigned long)tel.sats, tel.gpsFix ? 1 : 0, launched ? 1 : 0);
}

void loop() {
  static uint32_t nextSample = 0;
  static uint32_t lastFlush  = 0;
  const uint32_t periodMs = 1000UL / SAMPLE_HZ;

  server.handleClient();
  readGPS();

  uint32_t now = millis();
  if (now >= nextSample) {
    nextSample = now + periodMs;
    sampleSensors();
    detectLaunch();
    detectApogeeDeploy();
    logRow();
  }

  // flush SD periodically (and faster during flight) so a crash keeps data
  uint32_t flushEvery = launched ? 500 : 2000;
  if (haveSD && logFile && now - lastFlush > flushEvery) {
    logFile.flush();
    lastFlush = now;
  }
}
