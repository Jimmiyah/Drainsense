#include <SPI.h>
#include <LoRa.h>

// Sensor pins
const int trigPin = 5;
const int echoPin = 6;
const int tempPin = A1; // Temperature sensor analog pin
const int gasPin = A2; // MQ2 sensor analog pin
// Relay and fan settings
const int relayPin = 3; // Pin controlling the relay
const int fanPin = 4; // Pin controlling the fan
void setup() {
  Serial.begin(9600);

  // Initialize LoRa
  LoRa.setPins(SS, RST, DIO0);
  if (!LoRa.begin(915E6)) {
    Serial.println("LoRa initialization failed.");
    while (1);
  }

  // Initialize sensors
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
}

long readUltrasonicDistance() {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  long duration = pulseIn(echoPin, HIGH);
  long distance = (duration * 0.034) / 2;
  return distance;
}

float readTemperature() {
  int reading = analogRead(tempPin);
  float voltage = reading * 5.0 / 1023.0;
  float temperatureC = (voltage - 0.5) * 100;
  return temperatureC;
}

float readGasConcentration() {
  int reading = analogRead(gasPin);
  float concentration = reading * (5.0 / 1023.0);
  return concentration;
}

void loop() {
  long distance = readUltrasonicDistance();
  float temperature = readTemperature();
  float gasConcentration = readGasConcentration();
  // Control relay and fan based on sensor data
  if (gasValue > 100 || temp > 40) { // Thresholds to activate
    digitalWrite(relayPin, HIGH); // Turn on relay
    digitalWrite(fanPin, HIGH); // Turn on fan
  } else {
    digitalWrite(relayPin, LOW);
    digitalWrite(fanPin, LOW);
  }
  // Create a data packet
  String dataPacket = String(distance) + "," + String(temperature) + "," + String(gasConcentration);

  // Send data via LoRa
  LoRa.beginPacket();
  LoRa.print(dataPacket);
  LoRa.endPacket();

  delay(1000);
}
