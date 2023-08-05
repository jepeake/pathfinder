//////////////////////////////////////////////////////////////////////////////////////////
//                          Accelerometer Setups                               //
//////////////////////////////////////////////////////////////////////////////////////////

//server & gyro libraries
#include <Wire.h>
#include <MPU6050_light.h>
#include <MadgwickAHRS.h>
#include <HTTPClient.h>
#include <WiFi.h>
#include <freertos/task.h>


MPU6050 mpu(Wire);
Madgwick filter;

// MPU6050 calibration values
int16_t accelX_offset, accelY_offset, accelZ_offset;
int16_t gyroX_offset, gyroY_offset, gyroZ_offset;

// Constants for time and distance calculations
unsigned long previousTime;
float deltaTime;
float distanceX, distanceY, distanceZ;

const double wheelDiameter = 0.064; // in meters
const double stepsPerRevolution = 200.0;
const double wheelBase = 0.17; // in meters, distance between two wheels


//Orientation
float angleX;
float angleY = 0;
float angleZ;


//////////////////////////////////////////////////////////////////////////////////////////
//                          Server Setups                                               //
//////////////////////////////////////////////////////////////////////////////////////////
//**** WIFI+HTTP Variables *****

const char* ssid = "Kush";  //Wifi ID
const char* password = "kushkush";  //Password

unsigned long lastTime = 0;
unsigned long timerDelay = 500;

//Flags
String startFlag = "Wait";
String wallFlag = "left";

//Server Names
String ServerName = "http://172.20.10.2:3001/"; //local
//String ServerName = "http://44.202.132.63:3001"; //aws
String RoverData = ServerName + "RoverData";
String RoverControl = ServerName + "RoverControl";
String RoverStart = ServerName + "RoverStart";

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



//////////////////////////////////////////////////////////////////////////////////////////
//                          Vision & Motor Control Setups                               //
//////////////////////////////////////////////////////////////////////////////////////////

//Motor control library
#include <AccelStepper.h>


//pin definitions for FPGA<-->ESP32 UART communication
#define RXD2 16
#define TXD2 17

//Define width of image 
#define IMAGE_W 640

//Pin Definitions for steppers
#define MOTOR1_DIR_PIN 14
#define MOTOR1_STEP_PIN 4
#define MOTOR2_DIR_PIN 15
#define MOTOR2_STEP_PIN 2

// Create AccelStepper objects for the two motors
AccelStepper motor1(AccelStepper::DRIVER, MOTOR1_STEP_PIN, MOTOR1_DIR_PIN);
AccelStepper motor2(AccelStepper::DRIVER, MOTOR2_STEP_PIN, MOTOR2_DIR_PIN);

uint32_t L_SETPOINT = 140;
uint32_t R_SETPOINT = 500;

//not currently used
bool leftAvailable;
bool rightAvailable;

bool serverFollowMode = 0; // 0 = LEFT, 1 = RIGHT

float beaconW = 4; // 4 cm
float FOV = 70; // 70deg
float imageW = 640;

uint8_t byteCount = 0;
uint32_t data = 0;

//////////////////////////////////////////////////////////////////////////////////////////

TaskHandle_t serverTaskHandle;
TaskHandle_t UARTTaskHandle;


void setup() {
  
  // Motor & UART Setup ******************************************************************
  Serial.begin(115200);
  Serial2.begin(115200, SERIAL_8N1, RXD2, TXD2);

  // Set the maximum speed and acceleration of the motors
  motor1.setMaxSpeed(200.0);
  motor1.setAcceleration(100.0);
  motor2.setMaxSpeed(200.0);
  motor2.setAcceleration(100.0);

  // Set initial motor positions
  motor1.setCurrentPosition(0);
  motor2.setCurrentPosition(0);

   // Rover Setup *************************************************************************
  Wire.begin();
  mpu.begin();
  mpu.calcOffsets();

  filter.begin(10000); // Set filter update rate (in Hz), adjust as needed

  previousTime = micros();


  // **** IMU 2 CODE ****
  motor1.setMaxSpeed(200.0);
  motor1.setAcceleration(50.0);
  motor2.setMaxSpeed(200.0);
  motor2.setAcceleration(50.0);

  motor1.setCurrentPosition(0);
  motor2.setCurrentPosition(0);

  angleX = 0;
  angleY = 0;
  angleZ = 0;  
  
  //Rover Start *****************************************************************************
  WIFIconnect();
  RoverStartRequest();
  // **** IMU 2 CODE ****


  xTaskCreatePinnedToCore(
  serverFunction, /* Function to implement the task */
  "Server & IMU", /* Name of the task */
  10000,  /* Stack size in words */
  NULL,  /* Task input parameter */
  1,  /* Priority of the task */
  &serverTaskHandle,  /* Task handle. */
  0); /* Core where the task should run */

xTaskCreatePinnedToCore(
  readSerial, /* Function to implement the task */
  "UART & Motor Control", /* Name of the task */
  10000,  /* Stack size in words */
  NULL,  /* Task input parameter */
  1,  /* Priority of the task */
  &UARTTaskHandle,  /* Task handle. */
  1); /* Core where the task should run */

}


//////////////////////////////////////////////////////////////////////////////////////////
//                          Vision & Motor Functions                                    //
//////////////////////////////////////////////////////////////////////////////////////////

void distance(uint32_t red_width, uint32_t blue_width, uint32_t yellow_width){
  float FOVrad = radians(FOV);
  float redD = (beaconW * red_width) / (2 * imageW * tan(FOVrad / 2));
  float blueD = (beaconW * blue_width) / (2 * imageW * tan(FOVrad / 2));
  float yellowD = (beaconW * yellow_width) / (2 * imageW * tan(FOVrad / 2));
  Serial.println(redD);
  Serial.println(blueD);
  Serial.println(yellowD);
}

void fwd(int duration){
  Serial.println("GO FORWARD");

  motor1.setSpeed(100);
  motor2.setSpeed(100);

  motor1.runSpeed();
  motor2.runSpeed();
}

void right(int duration){
  duration = duration*10;
  Serial.println("TURN RIGHT BY: " + String(duration));

  motor1.setSpeed(40);
  motor2.setSpeed(-40);
  motor1.runSpeed();
  motor2.runSpeed();
}

void left(int duration){
  duration = duration*10;
  Serial.println("TURN LEFT BY: " + String(duration));
   // Set the target speed for both motors
  motor1.setSpeed(-40);
  motor2.setSpeed(40);
  motor1.runSpeed();
  motor2.runSpeed();
}

void follow(bool mode, uint32_t white_left_mid, uint32_t white_right_mid){
  int offset;
  
  if(mode){
    Serial.println("mode: right");
    uint32_t setpoint = R_SETPOINT; //set the setpoint as right edge - prefixed setpoint
    Serial.println("midpoint: " + String(white_right_mid));
    Serial.println("setpoint: " + String(setpoint));
    offset = -(setpoint - white_right_mid);
  }
  else{
    Serial.println("mode: left");
    //mode = 0, so left follow mode
    uint32_t setpoint = L_SETPOINT;
    Serial.println("midpoint: " + String(white_left_mid));
    Serial.println("setpoint: " + String(setpoint));
    offset = setpoint - white_left_mid;
    
  }
  Serial.println("offset: " + String(offset));

  
  if(abs(offset) > 120){
    if(mode == 0){//left follow
      Serial.println("Obstacle detected: TURN BIG RIGHT");
      leftAvailable = 0;
    }
    else{
      Serial.println("Obstacle detected: TURN BIG LEFT");
      rightAvailable = 0;
    }
  }
  else if(abs(offset) < 30){
    fwd(300);
//    Serial.print("GO FOWARD \n");
    leftAvailable = 1;
    rightAvailable = 1;
  }
  else if(offset > 0){
    if(mode){
      right(offset);
//      Serial.println("TURN RIGHT \n");
      rightAvailable = 1;
    }
    else{
      left(offset);
//      Serial.println("TURN LEFT \n");
      leftAvailable = 1;
    }
  }
  else if(offset < 0){
    if(mode){
      left(offset);
//      Serial.println("TURN LEFT \n");
      rightAvailable = 1;
    }
    else{
      right(offset);
      Serial.println("TURN RIGHT \n");
      leftAvailable = 1;
    }
     
  }
  
}

void readSerial(void * pvParameters){
  for(;;){
    uint32_t white_left_mid;
    uint32_t white_right_mid;
  
      while (Serial2.available()) {
        int x = Serial2.read();
        data = data | (x << (byteCount * 8));
        byteCount++;
    
        if (byteCount == 4) {
          uint32_t max = (data & 0x7ff);
          uint32_t min = (data >> 16) & 0x7ff;
          uint32_t xy = (data >> 11) & 0x1;
          uint32_t colour = (data >> 27) & 0x1f;
          uint32_t red_width;
          uint32_t blue_width;
          uint32_t yellow_width;
          // 1 = Red, 2 = Blue, 3 = Yellow, 4 = White LEFT, 5 = White RIGHT
    
    
          if(xy == 0 && colour == 1){ // x
            red_width = max - min;
          }
    
          if(xy == 0 && colour == 2){ // x
            blue_width = max - min;
          }
    
         if(xy == 0 && colour == 3){ // x
            yellow_width = max - min;
          }
    
         if(xy == 0 && colour == 4){ // x
            white_left_mid = (max + min) / 2;
           
          }
          
          if(xy == 0 && colour == 5){ // x
            white_right_mid = (max + min) / 2;
          }
    
          //distance(red_width, blue_width, yellow_width);
    
          data = 0;
          byteCount = 0;
        }
    }
    delay(1);
    serverFollowMode = (wallFlag == "right");
    follow(serverFollowMode, white_left_mid, white_right_mid);
  }
}




//////////////////////////////////////////////////////////////////////////////////////////
//                          Server & IMU Functions                                      //
//////////////////////////////////////////////////////////////////////////////////////////

//Function to make http requests
void httpRequests(float posX, float posY, float posZ, float roll, float pitch, float yaw) {
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
}

void serverFunction(void * pvParameters){
  for(;;){
    long prevLeftSteps = 0;
    long prevRightSteps = 0;
    float posX = 0, posY = 0, posZ = 0, theta = 0;

    mpu.update();

    // // **** ORIGINAL CODE ****
    // // Calculate delta time
    // unsigned long currentTime = micros();
    // deltaTime = (currentTime - previousTime) / 1000000.0; // Convert to seconds
    // previousTime = currentTime;
  
    // // Calculate distance traveled in each axis using dead reckoning
    // distanceX = (mpu.getGyroX() * deltaTime) + (0.5 * mpu.getAccX() * deltaTime * deltaTime);
    // distanceY = (mpu.getGyroY() * deltaTime) + (0.5 * mpu.getAccY() * deltaTime * deltaTime);
    // distanceZ = (mpu.getGyroZ() * deltaTime) + (0.5 * mpu.getAccZ() * deltaTime * deltaTime);
  
    // // Update robot's position
    // posX += distanceX;
    // posY += distanceY;
    // posZ += distanceZ;
  
    // // Update Madgwick filter with sensor data
    // filter.updateIMU(mpu.getGyroX(), mpu.getGyroY(), mpu.getGyroZ(), mpu.getAccX(), mpu.getAccY(), mpu.getAccZ());
  
    // // Get filtered orientation angles
    // float roll = filter.getRoll();
    // float pitch = filter.getPitch();
    // float yaw = filter.getYaw();
    // // **** ORIGINAL CODE ****

    // **** IMU 2 CODE ****
    //get orientation values
    angleX = mpu.getAngleX();
    angleY = mpu.getAngleY();
    angleZ = mpu.getAngleZ();

    long leftSteps = motor1.currentPosition();
    long rightSteps = motor2.currentPosition();

    float leftDistance = (leftSteps - prevLeftSteps) * (PI * wheelDiameter) / stepsPerRevolution;
    float rightDistance = (rightSteps - prevRightSteps) * (PI * wheelDiameter) / stepsPerRevolution;

    float distance = (leftDistance + rightDistance) / 2;

    float dTheta = (rightDistance - leftDistance) / wheelBase;

    posX += distance * cos(angleZ);
    posY += distance * sin(angleZ);

    prevLeftSteps = leftSteps;
    prevRightSteps = rightSteps;
    
    // Print position and orientation values
    Serial.println("Position: ");
    Serial.println(posX);
    Serial.println(posY);
    Serial.println("Orientation: ");
    Serial.println(angleZ);
    Serial.println("------------------");
    // **** IMU 2 CODE ****

    //HTTP Requests ***************************************************************************************    
    httpRequests(posX, posY, posZ, angleX, angleY, angleZ);

  }
}



//////////////////////////////////////////////////////////////////////////////////////////
//                            SETUP & LOOP                                              //
//////////////////////////////////////////////////////////////////////////////////////////

void loop() {
  //vTaskStartScheduler();
//  readSerial();
//  server();
}