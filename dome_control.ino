#include <SoftwareSerial.h>
#include <DHT.h>

// Pin configurations
#define FIRE_SENSOR_PIN 2    // Fire sensor pin
#define BUZZER_PIN 4         // Buzzer pin
#define RAIN_SENSOR A0       // Rain sensor pin
#define RAIN_THRESHOLD 500   // Adjust this threshold based on your sensor's calibration
#define MOTOR_IN1 7          // Motor 1 direction pin 1 (OUT1)
#define MOTOR_IN2 8          // Motor 1 direction pin 2 (OUT2)
#define MOTOR_ESA 5          // Motor 1 enable pin (PWM)
#define MOTOR_IN3 13         // Motor 2 direction pin 1 (OUT3)
#define MOTOR_IN4 12         // Motor 2 direction pin 2 (OUT4)
#define MOTOR_ESB 6          // Motor 2 enable pin (PWM)
#define DHT_PIN 9            // DHT sensor pin
#define DHT_TYPE DHT11       // DHT sensor type (DHT11 or DHT22)

// SoftwareSerial for GSM
SoftwareSerial gsmSerial(10, 11);  // RX, TX (for Arduino)

// DHT sensor
DHT dht(DHT_PIN, DHT_TYPE);

// Motor control flags and timers
unsigned long motorStartTime = 0;
unsigned long OPEN_CLOSE_TIME =15000; // Time (in ms) to open or close the roof fully
bool motorRunning = false;
bool messageSent = false;
bool r_Detected = false;
bool roof_Closed = false;  // Track the state of the roof

// Setup function
void setup() {
  // Set motor control pins
  pinMode(MOTOR_IN1, OUTPUT);
  pinMode(MOTOR_IN2, OUTPUT);
  pinMode(MOTOR_ESA, OUTPUT);
  pinMode(MOTOR_IN3, OUTPUT);
  pinMode(MOTOR_IN4, OUTPUT);
  pinMode(MOTOR_ESB, OUTPUT);

  // Set fire sensor and buzzer pins
  pinMode(FIRE_SENSOR_PIN, INPUT);
  pinMode(BUZZER_PIN, OUTPUT);

  // Set rain sensor pin
  pinMode(RAIN_SENSOR, INPUT);

  // Initialize the DHT sensor
  dht.begin();

  // Initialize serial communication
  Serial.begin(115200);    // For debugging
  gsmSerial.begin(9600); // GSM communication

  // Initialize motors in a stopped state
  // Initialize motors in a stopped state
digitalWrite(MOTOR_IN1, LOW);
digitalWrite(MOTOR_IN2, LOW);
analogWrite(MOTOR_ESA, 0);

digitalWrite(MOTOR_IN3, LOW);
digitalWrite(MOTOR_IN4, LOW);
analogWrite(MOTOR_ESB, 0);


  // GSM Initialization
  Serial.println("Initializing GSM module...");
  gsmSerial.println("AT");
  delay(1000);
  showResponse();

  // Check SIM card status
  gsmSerial.println("AT+CPIN?");
  delay(1000);
  showResponse();

  // Set SMS text mode
  gsmSerial.println("AT+CMGF=1");
  delay(1000);
  showResponse();

  Serial.println("Fire and Roof Control System Ready...");

  // Read the initial state of the rain sensor
  int r = analogRead(RAIN_SENSOR);
  if (r >= RAIN_THRESHOLD) {
    r_Detected = false;
    roof_Closed = false;
    Serial.println("Initial state: No Rain detected, keeping roof open.");
  } else {
    r_Detected = true;
    roof_Closed = true;
    Serial.println("Initial state: Rain detected, keeping roof closed.");
  }
}

// Loop function
void loop() {
  // Rain detection
  int rainValue = analogRead(RAIN_SENSOR);
  Serial.print("Rain Sensor Value: ");
  Serial.println(rainValue);

  // Fire detection
  int fireSensorValue = digitalRead(FIRE_SENSOR_PIN);
  if (fireSensorValue == LOW) {  // Fire detected
    Serial.println("Fire detected! Activating buzzer.");
    digitalWrite(BUZZER_PIN, HIGH);  // Activate buzzer
    if (roof_Closed == true) {
      motorStartTime = millis();
      motorRunning = true;
      r_Detected = false;
      roof_Closed = false;
      Serial.println("Opening the roof...");
      // Motor 1 rotates for opening
      digitalWrite(MOTOR_IN1, LOW);   // OUT1 low
      digitalWrite(MOTOR_IN2, HIGH);  // OUT2 high

      // Motor 2 rotates for opening (opposite direction with swapped wiring)
      digitalWrite(MOTOR_IN3, HIGH);  // OUT3 high
      digitalWrite(MOTOR_IN4, LOW);   // OUT4 low


    }
    if (!messageSent) {
      sendSOSMessage();  // Send SOS message only once
      messageSent = true;
    }
  } else {
    digitalWrite(BUZZER_PIN, LOW);
    messageSent = false;  // Reset message flag when no fire is detected
    Serial.println("Fire not detected.");
  }

  // Rain detection logic
  if (rainValue < RAIN_THRESHOLD && !r_Detected) {
    Serial.println("Rain detected. Closing the roof...");
    motorStartTime = millis();
    motorRunning = true;
    r_Detected = true;
    roof_Closed = true;
    // Motor 1 rotates for closing
    digitalWrite(MOTOR_IN1, HIGH);  // OUT1 high
    digitalWrite(MOTOR_IN2, LOW);   // OUT2 low

    // Motor 2 rotates for closing (opposite direction with swapped wiring)
    digitalWrite(MOTOR_IN3, LOW);   // OUT3 low
    digitalWrite(MOTOR_IN4, HIGH);  // OUT4 high


  }
  else if (rainValue >= RAIN_THRESHOLD && r_Detected) {
    Serial.println("No rain detected. Opening the roof...");
    motorStartTime = millis();
    motorRunning = true;
    r_Detected = false;
    roof_Closed = false;
    // Motor 1 rotates for opening
    digitalWrite(MOTOR_IN1, LOW);   // OUT1 low
    digitalWrite(MOTOR_IN2, HIGH);  // OUT2 high

    // Motor 2 rotates for opening (opposite direction with swapped wiring)
    digitalWrite(MOTOR_IN3, HIGH);  // OUT3 high
    digitalWrite(MOTOR_IN4, LOW);   // OUT4 low

  }

   // Serial commands for manual roof control
  if (Serial.available() > 0) {
    char a = Serial.read();
    Serial.print("Command received: ");
    Serial.println(a);

    if (a == 'a' && !roof_Closed) {
      Serial.println("Closing the roof...");
      motorStartTime = millis();
      motorRunning = true;
      roof_Closed = true;
      // Motor 1 rotates for closing
      digitalWrite(MOTOR_IN1, HIGH);  // OUT1 high
      digitalWrite(MOTOR_IN2, LOW);   // OUT2 low

      // Motor 2 rotates for closing (opposite direction with swapped wiring)
      digitalWrite(MOTOR_IN3, LOW);   // OUT3 low
      digitalWrite(MOTOR_IN4, HIGH);  // OUT4 high
    } else if (a == 'b') {
      Serial.println("Opening the roof...");
      motorStartTime = millis();
      motorRunning = true;
      roof_Closed = false;
      // Motor 1 rotates for opening
      digitalWrite(MOTOR_IN1, LOW);   // OUT1 low
      digitalWrite(MOTOR_IN2, HIGH);  // OUT2 high

      // Motor 2 rotates for opening (opposite direction with swapped wiring)
      digitalWrite(MOTOR_IN3, HIGH);  // OUT3 high
      digitalWrite(MOTOR_IN4, LOW);   // OUT4 low
    } 
  }


  // Ensure motor stops after moving for the specified time
  if (motorRunning && (millis() - motorStartTime < OPEN_CLOSE_TIME)) {
    analogWrite(MOTOR_ESA, 255); // Keep Motor 1 running
    analogWrite(MOTOR_ESB, 255); // Keep Motor 2 running
  } else if (motorRunning) {
  // Ensure motor stops after moving for the specified time
  if (millis() - motorStartTime >= OPEN_CLOSE_TIME) {
    // Stop Motors
    digitalWrite(MOTOR_IN1, LOW);
    digitalWrite(MOTOR_IN2, LOW);
    analogWrite(MOTOR_ESA, 0);

    digitalWrite(MOTOR_IN3, LOW);
    digitalWrite(MOTOR_IN4, LOW);
    analogWrite(MOTOR_ESB, 0);
    motorRunning = false;  // Stop the motor
    if (roof_Closed) {
      Serial.println("Roof closed, waiting for rain to stop.");
    } else {
      Serial.println("Roof opened, waiting for rain to start.");
    }
  }
}



  delay(500); // Short delay to prevent overloading the loop
}

// Function to send SOS message
void sendSOSMessage() {
  Serial.println("Sending SOS SMS...");
  gsmSerial.println("AT+CMGS=\"+916304764066\""); // Replace with your phone number
  delay(1000);
  gsmSerial.print("Fire Alert! Immediate action required.Location:Market yard ,latitude and longitude:16.2864° N, 80.4107° E");
  gsmSerial.write(26); // Ctrl+Z to send the SMS
  delay(3000);
  showResponse();
  Serial.println("SOS Message Sent Successfully!");
}

// Function to show GSM module response
void showResponse() {
  while (gsmSerial.available()) {
    Serial.write(gsmSerial.read());
  }
}
