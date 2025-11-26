#include <Arduino.h>
#include <esp32-hal-ledc.h>

#define ENA 25
#define ENB 26
#define IN1 13
#define IN2 12
#define IN3 14
#define IN4 27

#define IR_LEFT 18
#define IR_RIGHT 19
#define TRIG_PIN 4
#define ECHO_PIN 5
#define START_PIN 33
#define STOP_PIN 32

bool running = false;

void setup() {
    Serial.begin(115200);
    pinMode(IN1, OUTPUT); pinMode(IN2, OUTPUT);
    pinMode(IN3, OUTPUT); pinMode(IN4, OUTPUT);

    ledcAttach(ENA, 1000, 8);
    ledcAttach(ENB, 1000, 8);

    pinMode(IR_LEFT, INPUT);
    pinMode(IR_RIGHT, INPUT);
    pinMode(TRIG_PIN, OUTPUT);
    pinMode(ECHO_PIN, INPUT);
    pinMode(START_PIN, INPUT_PULLUP);
    pinMode(STOP_PIN, INPUT_PULLUP);
}

void loop() {
    if (!running) {
        if (digitalRead(START_PIN) == LOW) {
            Serial.println("Start pressed.");
            running = true;
            delay(500);
        }
        return;
    }

    if (digitalRead(STOP_PIN) == LOW) {
        Serial.println("Stop pressed.");
        running = false;
        stopMotors();
        delay(500);
        return;
    }

    int leftIR = digitalRead(IR_LEFT);     // LOW = wall, HIGH = clear
    int rightIR = digitalRead(IR_RIGHT);   // LOW = wall, HIGH = clear
    int distance = measureDistance();      // Front distance in cm

    Serial.print("Left IR: "); Serial.println(leftIR);
    Serial.print("Right IR: "); Serial.println(rightIR);
    Serial.print("Front Distance: "); Serial.println(distance);

    // ✅ Maze completion check
    // if (leftIR == HIGH && rightIR == HIGH && distance > 40) {
    //     Serial.println("Maze complete! Stopped. Press START to run again.");
    //     stopMotors();
    //     running = false;
    //     delay(500);
    //     return;
    // }

    // Left-Hand Rule: Prefer left turn if available
    if (leftIR == HIGH) {
        Serial.println("Turning left");
        turnLeft(120);
        delay(550);
        moveForward(90);
        delay(300);
    }
    else if (distance > 15) {
        Serial.println("Moving forward");
        moveForward(90);
        delay(400);
    }
    else if (rightIR == HIGH) {
        Serial.println("Turning right");
        turnRight(120);
        delay(550);
        moveForward(90);
        delay(300);
    }
    else if(leftIR == LOW && rightIR == LOW && distance < 15){
        // Dead-end: Turn around
        Serial.println("Dead-end detected, turning around");
        turnAround(130);
        delay(1400); // Delay adjusted for 180° turn
    }

    stopMotors();
    delay(100);
}

// Motor functions
void moveForward(int speed) {
    ledcWrite(ENA, speed);
    ledcWrite(ENB, speed);
    digitalWrite(IN1, HIGH); digitalWrite(IN2, LOW);
    digitalWrite(IN3, HIGH); digitalWrite(IN4, LOW);
}

void moveBackward(int speed) {
    ledcWrite(ENA, speed);
    ledcWrite(ENB, speed);
    digitalWrite(IN1, LOW); digitalWrite(IN2, HIGH);
    digitalWrite(IN3, LOW); digitalWrite(IN4, HIGH);
}

void turnLeft(int speed) {
    ledcWrite(ENA, speed);
    ledcWrite(ENB, speed);
    digitalWrite(IN1, LOW); digitalWrite(IN2, HIGH);
    digitalWrite(IN3, HIGH); digitalWrite(IN4, LOW);
}

void turnRight(int speed) {
    ledcWrite(ENA, speed);
    ledcWrite(ENB, speed);
    digitalWrite(IN1, HIGH); digitalWrite(IN2, LOW);
    digitalWrite(IN3, LOW); digitalWrite(IN4, HIGH);
}

void turnAround(int speed) {
    ledcWrite(ENA, speed);
    ledcWrite(ENB, speed);
    digitalWrite(IN1, LOW); digitalWrite(IN2, HIGH);
    digitalWrite(IN3, HIGH); digitalWrite(IN4, LOW);
}

void stopMotors() {
    ledcWrite(ENA, 0); ledcWrite(ENB, 0);
    digitalWrite(IN1, LOW); digitalWrite(IN2, LOW);
    digitalWrite(IN3, LOW); digitalWrite(IN4, LOW);
}

int measureDistance() {
    digitalWrite(TRIG_PIN, LOW);
    delayMicroseconds(2);
    digitalWrite(TRIG_PIN, HIGH);
    delayMicroseconds(10);
    digitalWrite(TRIG_PIN, LOW);
    
    long duration = pulseIn(ECHO_PIN, HIGH, 30000); // timeout 30ms
    int distance = duration * 0.034 / 2;
    return distance;
}
