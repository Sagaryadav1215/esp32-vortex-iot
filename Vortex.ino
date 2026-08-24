pre - Sample Code for Testing

/*************************************************************
  ESP32 VOICE & BLYNK IOT ROBOT + HOME AUTOMATION
  - Motors: L298N (Pins 27, 26, 25, 33)
  - Home: 4-Channel Relay (Pins 14, 12, 13, 23)
  - Sensors: DHT22 (Pin 4) & HC-SR04 (Pins 5, 18)
  - Voice: VC-02 via UART2 (Pins 16, 17)
 *************************************************************/

#define BLYNK_TEMPLATE_ID   "YOUR_TEMPLATE_ID"
#define BLYNK_TEMPLATE_NAME "YOUR_TEMPLATE_NAME"
#define BLYNK_AUTH_TOKEN    "YOUR_AUTH_TOKEN"

#define BLYNK_PRINT Serial
#include <WiFi.h>
#include <BlynkSimpleEsp32.h>
#include <DHT.h>

// WiFi Credentials
char ssid[] = "YOUR_WIFI_NAME";
char pass[] = "YOUR_WIFI_PASSWORD";

// Motor Pins
#define IN1 27
#define IN2 26
#define IN3 25
#define IN4 33

// Relay Pins (Renamed for clarity)
const int RELAY_PINS[] = {14, 12, 13, 23};

// Sensors
#define TRIG 5
#define ECHO 18
#define DHTPIN 4
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);

// Voice Module (UART2)
HardwareSerial voice(2);

unsigned long lastDHT = 0;

void setup() {
  Serial.begin(115200);
  voice.begin(9600, SERIAL_8N1, 16, 17); // RX=16, TX=17
 
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);

  pinMode(IN1, OUTPUT); pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT); pinMode(IN4, OUTPUT);
  pinMode(TRIG, OUTPUT); pinMode(ECHO, INPUT);

  for(int i=0; i<4; i++) {
    pinMode(RELAY_PINS[i], OUTPUT);
    digitalWrite(RELAY_PINS[i], HIGH); // Default OFF (Active Low)
  }

  dht.begin();
}

void loop() {
  Blynk.run();

  if (voice.available()) {
    int cmd = voice.read();
    executeVoice(cmd);
  }

  if (millis() - lastDHT > 2000) {
    sendDHT();
    lastDHT = millis();
  }
}

// --- Blynk App Controls ---
BLYNK_WRITE(V0) { digitalWrite(RELAY_PINS[0], param.asInt() ? LOW : HIGH); }
BLYNK_WRITE(V1) { digitalWrite(RELAY_PINS[1], param.asInt() ? LOW : HIGH); }
BLYNK_WRITE(V2) { digitalWrite(RELAY_PINS[2], param.asInt() ? LOW : HIGH); }
BLYNK_WRITE(V3) { digitalWrite(RELAY_PINS[3], param.asInt() ? LOW : HIGH); }

// --- Voice Command Logic ---
void executeVoice(int cmd) {
  // Safety: Obstacle check for forward command (cmd 1)
  if (cmd == 1 && getDistance() < 15) {
    stopRobot();
    return;
  }

  switch(cmd) {
    case 1: forward(); break;
    case 2: backward(); break;
    case 3: left(); break;
    case 4: right(); break;
    case 5: stopRobot(); break;
   
    // Relay Logic (6-13)
    case 6:  digitalWrite(RELAY_PINS[0], LOW);  break; // R1 ON
    case 7:  digitalWrite(RELAY_PINS[0], HIGH); break; // R1 OFF
    case 8:  digitalWrite(RELAY_PINS[1], LOW);  break; // R2 ON
    case 9:  digitalWrite(RELAY_PINS[1], HIGH); break; // R2 OFF
    case 10: digitalWrite(RELAY_PINS[2], LOW);  break; // R3 ON
    case 11: digitalWrite(RELAY_PINS[2], HIGH); break; // R3 OFF
    case 12: digitalWrite(RELAY_PINS[3], LOW);  break; // R4 ON
    case 13: digitalWrite(RELAY_PINS[3], HIGH); break; // R4 OFF
  }
}

// --- Sensor Helpers ---
int getDistance() {
  digitalWrite(TRIG, LOW); delayMicroseconds(2);
  digitalWrite(TRIG, HIGH); delayMicroseconds(10);
  digitalWrite(TRIG, LOW);
  return pulseIn(ECHO, HIGH, 25000) * 0.034 / 2;
}

void sendDHT() {
  float t = dht.readTemperature();
  float h = dht.readHumidity();
  if (!isnan(t)) { Blynk.virtualWrite(V5, t); Blynk.virtualWrite(V6, h); }
}

// --- Movement Helpers ---
void forward()  { digitalWrite(IN1, 1); digitalWrite(IN2, 0); digitalWrite(IN3, 1); digitalWrite(IN4, 0); }
void backward() { digitalWrite(IN1, 0); digitalWrite(IN2, 1); digitalWrite(IN3, 0); digitalWrite(IN4, 1); }
void left()     { digitalWrite(IN1, 0); digitalWrite(IN2, 1); digitalWrite(IN3, 1); digitalWrite(IN4, 0); }
void right()    { digitalWrite(IN1, 1); digitalWrite(IN2, 0); digitalWrite(IN3, 0); digitalWrite(IN4, 1); }
void stopRobot(){ digitalWrite(IN1, 0); digitalWrite(IN2, 0); digitalWrite(IN3, 0); digitalWrite(IN4, 0); }
