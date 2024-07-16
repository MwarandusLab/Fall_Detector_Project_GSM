#include <Wire.h>
#include <MPU6050.h>
#include <SoftwareSerial.h>

MPU6050 mpu;

int Buzzer = 8;
int RedLed = 9;
int GreenLed = 10;

int Sms = 0;
// Thresholds
const float ACC_THRESHOLD = 2.0; // Acceleration threshold for detecting fall
const float GYRO_THRESHOLD = 100.0; // Gyro threshold for detecting fall

//Create software serial object to communicate with SIM800L
SoftwareSerial mySerial(6, 7); //SIM800L Tx & Rx is connected to Arduino #6 & #7


void setup() {
  Serial.begin(9600);
  pinMode(Buzzer, OUTPUT);
  pinMode(RedLed, OUTPUT);
  pinMode(GreenLed, OUTPUT);

  digitalWrite(Buzzer, LOW);
  digitalWrite(GreenLed, HIGH);
  digitalWrite(RedLed, HIGH);

  Wire.begin();
  mpu.initialize();

  if (!mpu.testConnection()) {
    Serial.println("MPU6050 connection failed");
    while (1);
  }

  Serial.println("MPU6050 connection successful");

  //Begin serial communication with Arduino and SIM800L
  mySerial.begin(9600);

  Serial.println("Initializing..."); 
  delay(1000);

  mySerial.println("AT"); //Once the handshake test is successful, it will back to OK
  updateSerial();
  delay(1000);
}

void loop() {
  int16_t ax, ay, az;
  int16_t gx, gy, gz;

  // Read raw accelerometer and gyroscope data
  mpu.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);

  // Convert raw data to Gs and degrees/sec
  float axG = ax / 16384.0;
  float ayG = ay / 16384.0;
  float azG = az / 16384.0;

  float gxDPS = gx / 131.0;
  float gyDPS = gy / 131.0;
  float gzDPS = gz / 131.0;

  // Calculate the magnitude of acceleration
  float accMagnitude = sqrt(axG * axG + ayG * ayG + azG * azG);

  // Check if acceleration exceeds the threshold
  if (accMagnitude > ACC_THRESHOLD) {
    Serial.println("Possible fall detected (acceleration)");
  }

  // Check if gyroscope data exceeds the threshold
  if (abs(gxDPS) > GYRO_THRESHOLD || abs(gyDPS) > GYRO_THRESHOLD || abs(gzDPS) > GYRO_THRESHOLD) {
    Serial.println("Possible fall detected (gyroscope)");
    digitalWrite(GreenLed, LOW);
    digitalWrite(RedLed, HIGH);
    digitalWrite(Buzzer, HIGH);
    if(Sms == 0){
      SendMessage();
      Sms = 1;
    }
    delay(5000);
  }else{
    Sms = 0;
    digitalWrite(GreenLed, HIGH);
    digitalWrite(RedLed, LOW);
    digitalWrite(Buzzer, LOW);
  }

  // Print the sensor data
  Serial.print("Acc: ");
  Serial.print(axG); Serial.print(", ");
  Serial.print(ayG); Serial.print(", ");
  Serial.print(azG); Serial.print(" | ");
  Serial.print("Gyro: ");
  Serial.print(gxDPS); Serial.print(", ");
  Serial.print(gyDPS); Serial.print(", ");
  Serial.println(gzDPS);

  delay(200); // Adjust delay as necessary
}
void updateSerial()
{
  delay(500);
  while (Serial.available()) 
  {
    mySerial.write(Serial.read());//Forward what Serial received to Software Serial Port
  }
  while(mySerial.available()) 
  {
    Serial.write(mySerial.read());//Forward what Software Serial received to Serial Port
  }
}
void SendMessage(){
  mySerial.println("AT+CMGF=1"); // Configuring TEXT mode
  updateSerial();
  mySerial.println("AT+CMGS=\"+254715950674\"");//change ZZ with country code and xxxxxxxxxxx with phone number to sms
  updateSerial();
  mySerial.print("Alert Fall Detected"); //text content
  updateSerial();
  mySerial.write(26);
}