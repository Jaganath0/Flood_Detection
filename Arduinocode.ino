#include <LiquidCrystal_I2C.h>
#include <SoftwareSerial.h>

// Create software serial object to communicate with SIM900
SoftwareSerial mySerial(7, 8); // SIM900 Tx & Rx connected to Arduino #7 & #8

const int waterSensorPin = A0; // Pin where water level sensor is connected
const int threshold = 500;     // Define a threshold for water level
const int buzzerPin = 6;       // Pin where the buzzer is connected
const int redLedPin = 4;       // Pin where the red LED is connected
const int blueLedPin = 5;      // Pin where the blue LED is connected

// Initialize the LCD with the I2C address (commonly 0x27 or 0x3F)
LiquidCrystal_I2C lcd(0x27, 16, 2); // Adjust the address based on your I2C module

void setup() {
  // Begin serial communication with Arduino and Serial Monitor
  Serial.begin(9600);
  
  // Begin serial communication with SIM900
  mySerial.begin(9600);

  // Initialize pins as output
  pinMode(buzzerPin, OUTPUT);
  pinMode(redLedPin, OUTPUT);
  pinMode(blueLedPin, OUTPUT);

  // Default state: Blue LED is ON, others OFF
  digitalWrite(buzzerPin, LOW);
  digitalWrite(redLedPin, LOW);
  digitalWrite(blueLedPin, HIGH);

  // Initialize the LCD
  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("Initializing...");
  Serial.println("Initializing...");
  delay(1000);

  // Test SIM900 communication
  mySerial.println("AT"); // Handshaking with SIM900
  updateSerial();

  delay(2000); // Small delay to ensure SIM900 is ready
  lcd.clear();
}

void loop() {
  int waterLevel = analogRead(waterSensorPin); // Read water level sensor

  // Send water level as JSON data
  String json = "{\"waterLevel\": " + String(waterLevel) + "}";
  Serial.println(json);  // Send the water level in JSON format

  // Display water level on the LCD
  lcd.setCursor(0, 0);
  lcd.print("Water Level:    "); // Clear line
  lcd.setCursor(12, 0);
  lcd.print(waterLevel); // Display water level

  // Check if water level exceeds the threshold
  if (waterLevel > threshold) {
    String alertJson = "{\"alert\":\"Water level threshold exceeded!.....Flood alert\"}";
    Serial.println(alertJson);  // Send alert message as JSON

    // Activate flood alert indicators
    digitalWrite(buzzerPin, HIGH);
    digitalWrite(redLedPin, HIGH);
    digitalWrite(blueLedPin, LOW);
    lcd.setCursor(0, 1);
    lcd.print("Flood Alert   "); // Display normal status

    // Send SMS alert
    sendSMS("+9779840295109", "Flood Alert: Water level has reached a critical point."); // Replace with actual phone number

    delay(5000); // Wait before making the call

    // Initiate Flood Alert call
    Serial.println("Initiating Flood Alert call...");
    mySerial.println("ATD+9779840295109;"); // Replace with your phone number
    updateSerial();

    delay(20000); // Call for 20 seconds
    mySerial.println("ATH"); // Hang up
    updateSerial();
  } else {
    // Normal state
    String alertJson = "{\"alert\":\"Water level Normal!\"}";
    Serial.println(alertJson);  // Send alert message as JSON
    digitalWrite(buzzerPin, LOW);
    digitalWrite(redLedPin, LOW);
    digitalWrite(blueLedPin, HIGH);

    lcd.setCursor(0, 1);
    lcd.print("Level Normal   "); // Display normal status
  }

  delay(5000); // Wait 5 seconds before next check
}

void sendSMS(String phoneNumber, String message) {
  mySerial.println("AT+CMGF=1"); // Set SMS mode to text
  delay(1000);
  mySerial.println("AT+CMGS=\"" + phoneNumber + "\""); // Specify the phone number
  delay(1000);
  mySerial.println(message); // Send the alert message
  delay(1000);
  mySerial.write(26); // Send Ctrl+Z to send the SMS
  delay(3000); // Wait for SMS confirmation
}

void updateSerial() {
  while (Serial.available()) {
    mySerial.write(Serial.read()); // Forward Serial to Software Serial
  }
  while (mySerial.available()) {
    Serial.write(mySerial.read()); // Forward Software Serial to Serial
  }
}

