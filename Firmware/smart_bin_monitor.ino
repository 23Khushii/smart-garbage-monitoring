/*
 * Smart Garbage Monitoring System
 * Using NodeMCU ESP8266 + Ultrasonic Sensor + Blynk IoT
 *
 * Project by: Janvi Bajpai & Khushi Sahu
 * MIT-WPU Pune
 */

// ==================== LIBRARIES ====================
#define BLYNK_PRINT Serial
#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>

// ==================== BLYNK CREDENTIALS ====================
// Get these from Blynk App
char auth[] = "YOUR_BLYNK_AUTH_TOKEN";  // Replace with your Auth Token

// ==================== WIFI CREDENTIALS ====================
char ssid[] = "YOUR_WIFI_NAME";         // Replace with your WiFi name
char pass[] = "YOUR_WIFI_PASSWORD";     // Replace with your WiFi password

// ==================== PIN DEFINITIONS ====================
#define TRIG_PIN D1      // Ultrasonic Trigger pin
#define ECHO_PIN D2      // Ultrasonic Echo pin
#define LED_PIN  D4      // Optional LED for local indication
#define SWITCH_PIN D3    // Manual switch (optional)

// ==================== CONSTANTS ====================
#define BIN_HEIGHT 30.0          // Height of garbage bin in cm (adjust as needed)
#define FULL_THRESHOLD 80.0      // Percentage at which bin is considered full
#define NEAR_FULL_THRESHOLD 60.0 // Percentage at which bin is near full
#define EMPTY_THRESHOLD 20.0     // Percentage below which bin is considered empty

// ==================== GLOBAL VARIABLES ====================
float distance = 0.0;
float fillPercentage = 0.0;
bool alertSent = false;
bool nearFullAlertSent = false;
unsigned long lastMillis = 0;
const long interval = 2000;  // Read sensor every 2 seconds

// Blynk Virtual Pins
#define VPIN_DISTANCE    V0   // Distance in cm
#define VPIN_FILL_LEVEL  V1   // Fill level percentage
#define VPIN_STATUS      V2   // Status text
#define VPIN_ALERT       V3   // Alert notification
#define VPIN_GAUGE       V4   // Gauge widget

BlynkTimer timer;

// ==================== SETUP ====================
void setup() {
  Serial.begin(115200);
  delay(100);

  Serial.println("\n=================================");
  Serial.println("Smart Garbage Monitoring System");
  Serial.println("=================================");

  // Initialize pins
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  pinMode(LED_PIN, OUTPUT);
  pinMode(SWITCH_PIN, INPUT_PULLUP);

  digitalWrite(TRIG_PIN, LOW);
  digitalWrite(LED_PIN, LOW);

  // Connect to WiFi and Blynk
  Serial.print("Connecting to WiFi: ");
  Serial.println(ssid);

  Blynk.begin(auth, ssid, pass);

  // Setup timer to read sensor periodically
  timer.setInterval(interval, readSensor);

  Serial.println("System Ready!");
  Serial.println("Bin Height: " + String(BIN_HEIGHT) + " cm");
  Serial.println("Full Threshold: " + String(FULL_THRESHOLD) + "%");
}

// ==================== MAIN LOOP ====================
void loop() {
  Blynk.run();
  timer.run();

  // Check manual switch (optional)
  if (digitalRead(SWITCH_PIN) == LOW) {
    // Manual override - force read
    readSensor();
    delay(500);
  }
}

// ==================== SENSOR READING FUNCTION ====================
void readSensor() {
  // Send trigger pulse
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  // Read echo pulse (timeout after 30ms)
  long duration = pulseIn(ECHO_PIN, HIGH, 30000);

  // Calculate distance (speed of sound = 343 m/s = 0.0343 cm/µs)
  // Distance = (time × speed) / 2 (divide by 2 for round trip)
  distance = duration * 0.0343 / 2.0;

  // Validate reading
  if (distance <= 0 || distance > BIN_HEIGHT + 10) {
    Serial.println("Invalid sensor reading - retrying...");
    return;
  }

  // Calculate fill percentage
  // When bin is empty, distance = BIN_HEIGHT (sensor to bottom)
  // When bin is full, distance = 0 (sensor to garbage)
  fillPercentage = ((BIN_HEIGHT - distance) / BIN_HEIGHT) * 100.0;

  // Constrain between 0 and 100
  if (fillPercentage < 0) fillPercentage = 0;
  if (fillPercentage > 100) fillPercentage = 100;

  // Print to Serial Monitor
  Serial.println("---------------------------------");
  Serial.print("Distance: ");
  Serial.print(distance);
  Serial.println(" cm");
  Serial.print("Fill Level: ");
  Serial.print(fillPercentage);
  Serial.println(" %");

  // Determine status
  String status;
  if (fillPercentage >= FULL_THRESHOLD) {
    status = "BIN FULL - COLLECTION REQUIRED!";
    digitalWrite(LED_PIN, HIGH);  // LED ON when full

    // Send alert only once when bin becomes full
    if (!alertSent) {
      Blynk.logEvent("bin_full", "Garbage bin is FULL! Immediate collection required.");
      alertSent = true;
      nearFullAlertSent = true;  // Prevent near-full alert after full
      Serial.println(">>> ALERT: Bin is FULL! <<<");
    }
  }
  else if (fillPercentage >= NEAR_FULL_THRESHOLD) {
    status = "Bin is near full";
    digitalWrite(LED_PIN, LOW);

    if (!nearFullAlertSent && !alertSent) {
      Blynk.logEvent("bin_near_full", "Garbage bin is nearing full capacity. Plan collection soon.");
      nearFullAlertSent = true;
      Serial.println(">>> NOTICE: Bin is near full <<<");
    }
  }
  else if (fillPercentage <= EMPTY_THRESHOLD) {
    status = "Bin is empty";
    digitalWrite(LED_PIN, LOW);

    // Reset alerts when bin is emptied
    if (alertSent || nearFullAlertSent) {
      alertSent = false;
      nearFullAlertSent = false;
      Serial.println(">>> Bin has been emptied. Alerts reset. <<<");
    }
  }
  else {
    status = "Bin is partially filled";
    digitalWrite(LED_PIN, LOW);
    nearFullAlertSent = false;  // Reset near-full alert if level drops
  }

  Serial.print("Status: ");
  Serial.println(status);

  // Send data to Blynk
  Blynk.virtualWrite(VPIN_DISTANCE, distance);
  Blynk.virtualWrite(VPIN_FILL_LEVEL, fillPercentage);
  Blynk.virtualWrite(VPIN_STATUS, status);
  Blynk.virtualWrite(VPIN_GAUGE, fillPercentage);

  // Send alert to Blynk notification
  if (fillPercentage >= FULL_THRESHOLD && alertSent) {
    Blynk.virtualWrite(VPIN_ALERT, 255);  // Turn on alert LED widget
  } else {
    Blynk.virtualWrite(VPIN_ALERT, 0);    // Turn off alert LED widget
  }
}

// ==================== BLYNK CONNECTION HANDLER ====================
BLYNK_CONNECTED() {
  Serial.println("Connected to Blynk!");
  Blynk.syncAll();  // Sync all virtual pins
}

// ==================== MANUAL CONTROL FROM BLYNK APP ====================
BLYNK_WRITE(V5) {
  int pinValue = param.asInt();
  if (pinValue == 1) {
    Serial.println("Manual reading triggered from Blynk");
    readSensor();
  }
}

// ==================== OPTIONAL: RESET ALERTS ====================
BLYNK_WRITE(V6) {
  int resetValue = param.asInt();
  if (resetValue == 1) {
    alertSent = false;
    nearFullAlertSent = false;
    Serial.println("Alerts reset manually from Blynk");
  }
}
