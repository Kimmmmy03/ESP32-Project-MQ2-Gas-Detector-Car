#include <Arduino.h>

// ============================================================
// AUTOMATED MQ-2 GAS DETECTOR CAR
// ISB37503 - Realtime and Embedded System
// Universiti Kuala Lumpur (UniKL)
//
// Authors:
//   Chan Boon Hong       [52213123434]
//   Akmal Hakimi Bin Abd Rashid [52213123466]
//   Ti Hui Xian          [52215123002]
//
// Lecturer: Ts. Siti Fatimah Omar
// Date: 18/12/2025
// ============================================================

// ------------------------------------------------------------
// Pin Definitions
// ------------------------------------------------------------

// MQ-2 Gas Sensor (Analog Input)
const int GAS_SENSOR_PIN = 34;

// Buzzer
const int BUZZER_PIN = 27;

// L298N Motor Driver - Direction Pins
const int IN1 = 13;   // Left motor direction A
const int IN2 = 12;   // Left motor direction B
const int IN3 = 15;   // Right motor direction A
const int IN4 = 14;   // Right motor direction B

// L298N Motor Driver - Enable (PWM) Pins
const int ENA = 26;   // Left motor speed
const int ENB = 25;   // Right motor speed

// PWM Configuration (ESP32 LEDC)
const int pwmChannelA   = 0;
const int pwmChannelB   = 1;
const int pwmFreq       = 1000;   // 1 kHz
const int pwmResolution = 8;      // 8-bit (0–255)

// HC-SR04 Ultrasonic Sensors
// Front-Left
const int trigPin1 = 32;
const int echoPin1 = 33;
// Front-Right
const int trigPin2 = 2;
const int echoPin2 = 35;
// Rear-Left
const int trigPin3 = 16;
const int echoPin3 = 17;
// Rear-Right
const int trigPin4 = 4;
const int echoPin4 = 5;

// ------------------------------------------------------------
// Threshold Constants
// ------------------------------------------------------------

// MQ-2: normal air ~2600–2700, gas near sensor ~4095
// Alarm triggers at or above this value
const int GAS_THRESHOLD = 3200;

// Ultrasonic: obstacle detected if closer than this (cm)
const int OBSTACLE_DISTANCE = 20;

// Motor drive speed (0–255)
const int DRIVE_SPEED = 200;

// ============================================================
// Utility: Measure distance with HC-SR04 (returns cm)
// Returns 999 if no echo received (i.e., path is clear)
// ============================================================
long measureDistance(int trigPin, int echoPin) {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  long duration = pulseIn(echoPin, HIGH, 30000); // 30 ms timeout
  if (duration == 0) return 999;                 // no echo = clear

  return duration * 0.034 / 2;                   // convert to cm
}

// ============================================================
// Low-Level Motor Control
// ============================================================
void setMotorA(bool forward, int speed) {
  digitalWrite(IN1, forward ? HIGH : LOW);
  digitalWrite(IN2, forward ? LOW  : HIGH);
  ledcWrite(pwmChannelA, speed);
}

void setMotorB(bool forward, int speed) {
  digitalWrite(IN3, forward ? HIGH : LOW);
  digitalWrite(IN4, forward ? LOW  : HIGH);
  ledcWrite(pwmChannelB, speed);
}

void stopMotors() {
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, LOW);
  ledcWrite(pwmChannelA, 0);
  ledcWrite(pwmChannelB, 0);
}

// ============================================================
// High-Level Movement Helpers
// ============================================================
void driveForward() {
  setMotorA(true,  DRIVE_SPEED);
  setMotorB(true,  DRIVE_SPEED);
}

void driveBackward() {
  setMotorA(false, DRIVE_SPEED);
  setMotorB(false, DRIVE_SPEED);
}

void turnLeft() {
  // Left motor backward, right motor forward → rotates left
  setMotorA(false, DRIVE_SPEED);
  setMotorB(true,  DRIVE_SPEED);
}

void turnRight() {
  // Left motor forward, right motor backward → rotates right
  setMotorA(true,  DRIVE_SPEED);
  setMotorB(false, DRIVE_SPEED);
}

// ============================================================
// Gas Detection
// Returns true when MQ-2 reading >= GAS_THRESHOLD
// ============================================================
bool gasDetected() {
  int gasValue = analogRead(GAS_SENSOR_PIN);
  Serial.print("Gas sensor value: ");
  Serial.print(gasValue);
  Serial.print(" | Threshold: ");
  Serial.println(GAS_THRESHOLD);
  return gasValue >= GAS_THRESHOLD;
}

// ============================================================
// setup()
// ============================================================
void setup() {
  Serial.begin(115200);

  // Extend ADC range to full 3.3 V for MQ-2
  analogSetPinAttenuation(GAS_SENSOR_PIN, ADC_11db);
  pinMode(GAS_SENSOR_PIN, INPUT);

  // Motor direction pins
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);

  // PWM channels for motor speed
  ledcSetup(pwmChannelA, pwmFreq, pwmResolution);
  ledcSetup(pwmChannelB, pwmFreq, pwmResolution);
  ledcAttachPin(ENA, pwmChannelA);
  ledcAttachPin(ENB, pwmChannelB);

  // Buzzer
  pinMode(BUZZER_PIN, OUTPUT);

  // Ultrasonic sensors
  pinMode(trigPin1, OUTPUT); pinMode(echoPin1, INPUT);
  pinMode(trigPin2, OUTPUT); pinMode(echoPin2, INPUT);
  pinMode(trigPin3, OUTPUT); pinMode(echoPin3, INPUT);
  pinMode(trigPin4, OUTPUT); pinMode(echoPin4, INPUT);

  // Safe startup state
  stopMotors();
  digitalWrite(BUZZER_PIN, LOW);

  Serial.println("=== Gas Detector Car Initialized ===");
}

// ============================================================
// loop() — Main Control Loop
//
// Priority order:
//   1. Gas detection  → stop + buzzer (highest priority)
//   2. Obstacle avoidance → steer around obstacles
//   3. Default roaming  → drive forward
// ============================================================
void loop() {
  // --- 1. Read all four ultrasonic sensors ---
  long dFrontLeft  = measureDistance(trigPin1, echoPin1);
  long dFrontRight = measureDistance(trigPin2, echoPin2);
  long dRearLeft   = measureDistance(trigPin3, echoPin3);
  long dRearRight  = measureDistance(trigPin4, echoPin4);

  Serial.print("FL: "); Serial.print(dFrontLeft);
  Serial.print("  FR: "); Serial.print(dFrontRight);
  Serial.print("  RL: "); Serial.print(dRearLeft);
  Serial.print("  RR: "); Serial.println(dRearRight);

  bool frontLeftBlocked  = (dFrontLeft  < OBSTACLE_DISTANCE);
  bool frontRightBlocked = (dFrontRight < OBSTACLE_DISTANCE);
  bool rearLeftBlocked   = (dRearLeft   < OBSTACLE_DISTANCE);
  bool rearRightBlocked  = (dRearRight  < OBSTACLE_DISTANCE);

  // --- 2. Gas check (HIGHEST PRIORITY) ---
  if (gasDetected()) {
    stopMotors();
    digitalWrite(BUZZER_PIN, HIGH);
    Serial.println(">>> GAS DETECTED — STOP & ALARM <<<");
    delay(100);
    return;   // skip obstacle/movement logic this cycle
  } else {
    digitalWrite(BUZZER_PIN, LOW);
  }

  // --- 3. Obstacle avoidance logic ---
  if (frontLeftBlocked || frontRightBlocked ||
      rearLeftBlocked  || rearRightBlocked) {

    if (frontLeftBlocked && !frontRightBlocked) {
      Serial.println("Obstacle front-LEFT  → turning RIGHT");
      turnRight();

    } else if (frontRightBlocked && !frontLeftBlocked) {
      Serial.println("Obstacle front-RIGHT → turning LEFT");
      turnLeft();

    } else if (frontLeftBlocked && frontRightBlocked) {
      // Both front sides blocked
      if (!rearLeftBlocked && !rearRightBlocked) {
        Serial.println("Both front blocked, rear clear → reversing");
        driveBackward();
      } else {
        // Completely surrounded → rotate in place to find a way out
        Serial.println("Front & rear blocked → rotating in place");
        turnLeft();
      }

    } else {
      // Rear obstacle only — front is clear, keep going
      Serial.println("Rear obstacle only → moving forward");
      driveForward();
    }

  } else {
    // --- 4. No obstacles and no gas → roam freely ---
    Serial.println("Path clear → moving forward");
    driveForward();
  }

  delay(100); // loop stabilisation delay
}
