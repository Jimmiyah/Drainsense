#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_MPU6050.h>
#include <LoRa.h>

// OLED display settings
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET    -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// MPU6050 settings
Adafruit_MPU6050 mpu;

// LDR settings
const int ldrPin = A0; // LDR pin
const int laserPin = 2; // Laser pin

// Ultrasonic sensor settings
const int trigPin = 3;
const int echoPin = 4;

// LoRa settings
#define SS 10
#define RST 9
#define DIO0 2

void setup() {
  // Initialize serial communication
  Serial.begin(9600);

  // Initialize OLED display
  if (!display.begin(SSD1306_I2C_ADDRESS, OLED_RESET)) {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;);
  }
  display.clearDisplay();
  
  // Initialize MPU6050
  if (!mpu.begin()) {
    Serial.println("Failed to find MPU6050 chip");
    while (1) { delay(10); }
  }

  // Initialize LDR and laser
  pinMode(ldrPin, INPUT);
  pinMode(laserPin, OUTPUT);
  digitalWrite(laserPin, HIGH); // Turn on laser

  // Initialize Ultrasonic sensor
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);

  // Initialize LoRa
  LoRa.setPins(SS, RST, DIO0);
  if (!LoRa.begin(915E6)) {
    Serial.println("LoRa initialization failed.");
    while (1);
  }
}

void loop() {
  // Read MPU6050 data
  sensors_event_t a, g, temp;
  mpu.getEvent(&a, &g, &temp);

  // Display MPU6050 data on OLED
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.print("Accel X: "); display.println(a.acceleration.x);
  display.print("Accel Y: "); display.println(a.acceleration.y);
  display.print("Accel Z: "); display.println(a.acceleration.z);
  
  // Read LDR value
  int ldrValue = analogRead(ldrPin);
  display.print("LDR Value: "); display.println(ldrValue);

  // Measure distance using Ultrasonic sensor
  long duration, distance;
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  duration = pulseIn(echoPin, HIGH);
  distance = (duration * 0.034 / 2); // Distance in cm

  display.print("Water Level: ");
  display.print(distance);
  display.println(" cm");

  // Check if drain is open or flooding
  if (ldrValue > 500) { // Threshold for detecting waste
    display.print("Alert: Waste Detected!");
  } else if (distance < 20) { // Threshold for water level
    display.print("Alert: High Water Level!");
  } else {
    display.print("Drain Status: Normal");
  }

  display.display();

  // Try to parse packet
  int packetSize = LoRa.parsePacket();
  if (packetSize) {
    // Received a packet
    String dataPacket = LoRa.readString();
    Serial.print("Received data: ");
    Serial.println(dataPacket);

    // Split the received data into components
    int firstComma = dataPacket.indexOf(',');
    int secondComma = dataPacket.indexOf(',', firstComma + 1);
    String distanceStr = dataPacket.substring(0, firstComma);
    String temperatureStr = dataPacket.substring(firstComma + 1, secondComma);
    String gasConcentrationStr = dataPacket.substring(secondComma + 1);

    long distance = distanceStr.toInt();          // Water level in cm
    float temperature = temperatureStr.toFloat(); // Temperature in Celsius
    float gasConcentration = gasConcentrationStr.toFloat(); // Gas concentration

    // Display the received data on the OLED
    display.clearDisplay();
    display.setTextSize(1);    // Normal 1:1 pixel scale
    display.setTextColor(SSD1306_WHITE); // Draw white text
    display.setCursor(0, 0);   // Start at top-left corner

    display.print("Water Level: ");
    display.print(distance);
    display.println(" cm");

    display.print("Temp: ");
    display.print(temperature);
    display.println(" C");

    display.print("Gas: ");
    display.print(gasConcentration);
    display.println(" ppm");

    display.display(); // Show on OLED
  }

  delay(1000); // Delay between receiving packets
}

  delay(1000); // Delay for next cycle
}
