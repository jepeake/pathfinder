#include <Wire.h>
#include <MPU6050_light.h>
#include <AccelStepper.h>

#include <HTTPClient.h>
#include <WiFi.h>


#define MOTOR1_DIR_PIN 14
#define MOTOR1_STEP_PIN 4
#define MOTOR2_DIR_PIN 15
#define MOTOR2_STEP_PIN 2

AccelStepper stepperLeft(AccelStepper::DRIVER, MOTOR1_STEP_PIN, MOTOR1_DIR_PIN); //motor1
AccelStepper stepperRight(AccelStepper::DRIVER, MOTOR2_STEP_PIN, MOTOR2_DIR_PIN); //motor2

int16_t accelX_offset, accelY_offset, accelZ_offset;
int16_t gyroX_offset, gyroY_offset, gyroZ_offset;

const double wheelDiameter = 0.064; // in meters
const double stepsPerRevolution = 200.0;
const double wheelBase = 0.17; // in meters, distance between two wheels

float angleX;
float angleY;
float angleZ;

MPU6050 imu(Wire);

// **** WiFI + Http Stuff ****

//HOLD BOOT BUTTON DOWN DURING UPLOAD
//SET MODULE TO ESP32-WROOM DA MODULE
//CONNECT LAPTOP TO MOBILE HOTSPOT ASWELL SO ESP CAN FIND IP ADDRESS
const char* ssid = "gobee";  //Wifi ID
const char* password = "123456789";  //Password

unsigned long lastTime = 0;
unsigned long timerDelay = 500;

//Server Names
String ServerName = "http://172.20.10.2:3001/"; //local
//String ServerName = "http://44.202.132.63:3001";
String RoverData = ServerName + "RoverData";
String RoverControl = ServerName + "RoverControl";
String RoverStart = ServerName + "RoverStart";


//Flags
String startFlag = "Wait";
String wallFlag = "left";

//WiFI Connection Function
void WIFIconnect() {
  WiFi.begin(ssid, password);
  Serial.println("Connecting");
  while(WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to WiFi network with IP Address: ");
  Serial.println(WiFi.localIP());

  Serial.println("Timer set to 1 second (timerDelay variable), it will take 1 seconds before publishing the first reading.");
  delay(1000);
}

//Rover Start Function
void RoverStartRequest() {
  WiFiClient client;
  HTTPClient http;

  while (startFlag == "Wait") {
  Serial.println("Getting Rover Start Flag");
  http.begin(client, RoverStart);
  int httpResponseCodeStart = http.GET();
  String payload = http.getString();
  Serial.print("HTTP GET Response code: " + String(httpResponseCodeStart) + "\n");
  Serial.print("HTTP GET Response: " + payload + "\n");
  startFlag = payload;
  http.end();
  }
  Serial.println("Rover Starting");
}


void setup() {
  Serial.begin(115200);
  Wire.begin();
  imu.begin();
  imu.calcOffsets(); 
  
  stepperLeft.setMaxSpeed(200.0);
  stepperLeft.setAcceleration(50.0);
  stepperRight.setMaxSpeed(200.0);
  stepperRight.setAcceleration(50.0);

  stepperLeft.setCurrentPosition(0);
  stepperRight.setCurrentPosition(0);

  angleX = 0;
  angleY = 0;
  angleZ = 0;

  WIFIconnect();
  RoverStartRequest();
}

//Function to make http requests
void httpRequests(float posX, float posY, float posZ, float roll, float pitch, float yaw) {
  //  if ((millis() - lastTime) > timerDelay) {
    
  //Check WiFi connection status
  if(WiFi.status()== WL_CONNECTED){
    WiFiClient client;
    HTTPClient http;

    //POST Rover Data
    Serial.println("Sending Rover Data");
    http.begin(client, RoverData);
    http.addHeader("Content-Type", "application/json");
    int httpResponseCodeP = http.POST("{\"x\":\"" + String(posX) + "\",\"y\":\"" + String(posY) +  "\",\"z\":\"" + String(posZ) + "\",\"Roll\":\"" + String(roll) +  "\",\"orientation\":\"" + String(pitch) +  "\",\"Yaw\":\"" + String(yaw) + "\"}");
    Serial.print("HTTP POST Response code: " + String(httpResponseCodeP)+ "\n");
    http.end();

    //GET Rover Control Instructions
    Serial.println("Getting Rover Instructions");
    http.begin(client, RoverControl);
    int httpResponseCodeG = http.GET();
    String payload = http.getString();
    Serial.print("HTTP GET Response code: " + String(httpResponseCodeG) + "\n");
    Serial.print("HTTP GET Response: " + payload + "\n");
    wallFlag = payload;
    http.end();

  }

  else {
    Serial.println("WiFi Disconnected - Cannot Send Position");
  }

  //   lastTime = millis();
  //  }
}

void fwd(){
  stepperLeft.setSpeed(200);
  stepperRight.setSpeed(200);

  stepperLeft.runSpeed();
  stepperRight.runSpeed();
}

void loop() {
  long prevLeftSteps = 0;
  long prevRightSteps = 0;
  float posX = 0, posY = 0, posZ = 0, theta = 0;

  imu.update();

  angleX = imu.getAngleX();
  angleY = imu.getAngleY();
  angleZ = imu.getAngleZ();

  long leftSteps = stepperLeft.currentPosition();
  long rightSteps = stepperRight.currentPosition();

  float leftDistance = (leftSteps - prevLeftSteps) * (PI * wheelDiameter) / stepsPerRevolution;
  float rightDistance = (rightSteps - prevRightSteps) * (PI * wheelDiameter) / stepsPerRevolution;

  float distance = (leftDistance + rightDistance) / 2;

  float dTheta = (rightDistance - leftDistance) / wheelBase;

  posX += distance * cos(theta + dTheta/2);
  posY += distance * sin(theta + dTheta/2);
  theta += dTheta;

  prevLeftSteps = leftSteps;
  prevRightSteps = rightSteps;

  Serial.println("Position: ");
  Serial.println(posX);
  Serial.println(posY);
  Serial.println(posZ);
  Serial.println("Orientation: ");
  Serial.println(angleX);
  Serial.println(angleY);
  Serial.println(angleZ);
  Serial.println("------------------");

  fwd();

  httpRequests(posX, posY, posZ, angleX, angleY, angleZ);
}
