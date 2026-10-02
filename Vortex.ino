#define BLYNK_TEMPLATE_ID   "YOUR_TEMPLATE_ID"
#define BLYNK_TEMPLATE_NAME "YOUR_TEMPLATE_NAME"
#define BLYNK_AUTH_TOKEN    "YOUR_AUTH_TOKEN"

#define BLYNK_PRINT Serial
#include <WiFi.h>
#include <BlynkSimpleEsp32.h>
#include <DHT.h>

// WiFi credentials - replace with your network details
char ssid[] = "YOUR_WIFI_NAME";
char pass[] = "YOUR_WIFI_PASSWORD";

// Motor pins
#define IN1 27
#define IN2 26
#define IN3 25
#define IN4 33

// Relay pins: default OFF (active low relay modules)
const int RELAY_PINS[] = {14, 12, 13, 23};

// Sensors
#define TRIG 5
#define ECHO 18
#define DHTPIN 4
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);

// Voice module UART2 (RX=16, TX=17)
HardwareSerial voice(2);

unsigned long lastDHT = 0;
const uint32_t SENSOR_READ_INTERVAL = 2000;

void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println("\nVORTEX booting...");

  voice.begin(9600, SERIAL_8N1, 16, 17);

  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);
  pinMode(TRIG, OUTPUT);
  pinMode(ECHO, INPUT);

  for (int i = 0; i < 4; i++) {
    pinMode(RELAY_PINS[i], OUTPUT);
    digitalWrite(RELAY_PINS[i], HIGH);  // default OFF for active-low relay boards
  }

  dht.begin();

  // Connect to WiFi before starting Blynk
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, pass);

  Serial.print("Connecting to WiFi");
  unsigned long wifiStart = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - wifiStart < 20000) {
    delay(500);
    Serial.print('.');
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println();
    Serial.print("WiFi connected. IP: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println();
    Serial.println("WiFi connection failed. Check SSID/password.");
  }

  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);
  Serial.println("Blynk service started.");

  stopRobot();
}

void loop() {
  Blynk.run();

  if (voice.available()) {
    int cmd = voice.read();
    Serial.print("Voice command received: ");
    Serial.println(cmd);
    executeVoice(cmd);
  }

  if (millis() - lastDHT >= SENSOR_READ_INTERVAL) {
    sendDHT();
    lastDHT = millis();
  }
}

// --- Blynk App Controls ---
BLYNK_WRITE(V0) {
  bool state = param.asInt() == 1;
  digitalWrite(RELAY_PINS[0], state ? LOW : HIGH);
}

BLYNK_WRITE(V1) {
  bool state = param.asInt() == 1;
  digitalWrite(RELAY_PINS[1], state ? LOW : HIGH);
}

BLYNK_WRITE(V2) {
  bool state = param.asInt() == 1;
  digitalWrite(RELAY_PINS[2], state ? LOW : HIGH);
}

BLYNK_WRITE(V3) {
  bool state = param.asInt() == 1;
  digitalWrite(RELAY_PINS[3], state ? LOW : HIGH);
}

// --- Voice Command Logic ---
void executeVoice(int cmd) {
  // Safety: prevent driving into obstacle in forward direction
  if (cmd == 1 && getDistance() < 15) {
    Serial.println("Obstacle detected. Movement blocked.");
    stopRobot();
    return;
  }

  switch (cmd) {
    case 1:
      forward();
      break;
    case 2:
      backward();
      break;
    case 3:
      left();
      break;
    case 4:
      right();
      break;
    case 5:
      stopRobot();
      break;

    // Relay logic (6-13)
    case 6:
      digitalWrite(RELAY_PINS[0], LOW);
      break;  // R1 ON
    case 7:
      digitalWrite(RELAY_PINS[0], HIGH);
      break; // R1 OFF
    case 8:
      digitalWrite(RELAY_PINS[1], LOW);
      break;  // R2 ON
    case 9:
      digitalWrite(RELAY_PINS[1], HIGH);
      break; // R2 OFF
    case 10:
      digitalWrite(RELAY_PINS[2], LOW);
      break;  // R3 ON
    case 11:
      digitalWrite(RELAY_PINS[2], HIGH);
      break; // R3 OFF
    case 12:
      digitalWrite(RELAY_PINS[3], LOW);
      break;  // R4 ON
    case 13:
      digitalWrite(RELAY_PINS[3], HIGH);
      break; // R4 OFF
    default:
      Serial.print("Unknown command: ");
      Serial.println(cmd);
      break;
  }
}

// --- Sensor Helpers ---
int getDistance() {
  digitalWrite(TRIG, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG, LOW);

  long duration = pulseIn(ECHO, HIGH, 25000);
  if (duration == 0) {
    Serial.println("Ultrasonic sensor timeout.");
    return 999;
  }

  float distanceCm = duration * 0.034 / 2.0;
  return (int)distanceCm;
}

void sendDHT() {
  float temp = dht.readTemperature();
  float humidity = dht.readHumidity();

  if (isnan(temp) || isnan(humidity)) {
    Serial.println("DHT read failed.");
    return;
  }

  Blynk.virtualWrite(V5, temp);
  Blynk.virtualWrite(V6, humidity);

  Serial.print("Temperature: ");
  Serial.print(temp);
  Serial.print("C, Humidity: ");
  Serial.print(humidity);
  Serial.println("%");
}

// --- Movement Helpers ---
void forward() {
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, LOW);
}

void backward() {
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, HIGH);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, HIGH);
}

void left() {
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, HIGH);
  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, LOW);
}

void right() {
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, HIGH);
}

void stopRobot() {
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, LOW);
}
