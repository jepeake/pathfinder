#include <Wire.h>
#include <MPU6050_light.h>
#include <AccelStepper.h>
#include <MadgwickAHRS.h>
#include <HTTPClient.h>
#include <WiFi.h>
#include <freertos/task.h>

#define IMAGE_W 640

#define RXD2 16
#define TXD2 17

#define MOTOR1_DIR_PIN 14
#define MOTOR1_STEP_PIN 4
#define MOTOR2_DIR_PIN 15
#define MOTOR2_STEP_PIN 2

uint32_t L_SETPOINT = 110;
uint32_t R_SETPOINT = 540;

bool leftAvailable;
bool rightAvailable;

bool serverFollowMode = 0; // 0 = LEFT, 1 = RIGHT

float beaconW = 4; // 4 cm
float FOV = 70; // 70deg
float imageW = 640;

uint8_t byteCount = 0;
uint32_t data = 0;

SemaphoreHandle_t mutex;

TaskHandle_t serverTaskHandle;
TaskHandle_t UARTTaskHandle;

AccelStepper motor1(AccelStepper::DRIVER, MOTOR1_STEP_PIN, MOTOR1_DIR_PIN); //left
AccelStepper motor2(AccelStepper::DRIVER, MOTOR2_STEP_PIN, MOTOR2_DIR_PIN); //right

int16_t accelX_offset, accelY_offset, accelZ_offset;
int16_t gyroX_offset, gyroY_offset, gyroZ_offset;

const double wheelDiameter = 0.064; // in meters
const double stepsPerRevolution = 200.0;
const double wheelBase = 0.17; // in meters, distance between two wheels

MPU6050 imu(Wire);
Madgwick filter;

const char* ssid = "gobee";  //Wifi ID
const char* password = "123456789";  //Password

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
  Serial2.begin(115200, SERIAL_8N1, RXD2, TXD2);
  Wire.begin();
  imu.begin();
  imu.calcOffsets(); 

  mutex = xSemaphoreCreateMutex();
  
  motor1.setMaxSpeed(200.0);
  motor1.setAcceleration(50.0);
  motor2.setMaxSpeed(200.0);
  motor2.setAcceleration(50.0);

  motor1.setCurrentPosition(0);
  motor2.setCurrentPosition(0);

  filter.begin(1000); 

  WIFIconnect();
  RoverStartRequest();

  xTaskCreatePinnedToCore(
  serverFunction, /* Function to implement the task */
  "Server & IMU", /* Name of the task */
  10000,  /* Stack size in words */
  NULL,  /* Task input parameter */
  2,  /* Priority of the task */
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

void distance(uint32_t red_width, uint32_t blue_width, uint32_t yellow_width){
  float FOVrad = radians(FOV);
  float redD = (beaconW * red_width) / (2 * imageW * tan(FOVrad / 2));
  float blueD = (beaconW * blue_width) / (2 * imageW * tan(FOVrad / 2));
  float yellowD = (beaconW * yellow_width) / (2 * imageW * tan(FOVrad / 2));
  Serial.println(redD);
  Serial.println(blueD);
  Serial.println(yellowD);
}

void fwd(){
  motor1.setSpeed(200);
  motor2.setSpeed(200);

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
    fwd();
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
    if (xSemaphoreTake(mutex, portMAX_DELAY)) {
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
          xSemaphoreGive(mutex);
        }
    }
    vTaskDelay(1);
    follow(serverFollowMode, white_left_mid, white_right_mid);  
    }
 }
}

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


void serverFunction(void * pvParameters) {
  for(;;) {

if (xSemaphoreTake(mutex, portMAX_DELAY)) {
    long prevLeftSteps = 0;
    long prevRightSteps = 0;
    float posX = 0, posY = 0;

    imu.update();
  
    filter.updateIMU(imu.getGyroX(), imu.getGyroY(), imu.getGyroZ(), imu.getAccX(), imu.getAccY(), imu.getAccZ());

    float yaw = (filter.getYaw()) * (PI/180);

    long leftSteps = motor1.currentPosition();
    long rightSteps = motor2.currentPosition();

    float leftDistance = (leftSteps - prevLeftSteps) * (PI * wheelDiameter) / stepsPerRevolution;
    float rightDistance = (rightSteps - prevRightSteps) * (PI * wheelDiameter) / stepsPerRevolution;

    float distance = (leftDistance + rightDistance) / 2;

    posX += distance * cos(yaw);
    posY += distance * sin(yaw);

    prevLeftSteps = leftSteps;
    prevRightSteps = rightSteps;

    Serial.println("Position: ");
    Serial.println(posX);
    Serial.println(posY);
    Serial.println("Orientation: ");
    Serial.println(yaw);
    Serial.println("------------------");

        httpRequests(posX, posY, posY, yaw, yaw, yaw);
        xSemaphoreGive(mutex);
        }
        vTaskDelay(1);
    }
}

void loop() {
  //vTaskStartScheduler();
//  readSerial();
//  server();
}
