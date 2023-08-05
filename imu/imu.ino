//#include <Wire.h>
//#include <MPU6050_light.h>
//#include <MadgwickAHRS.h>
//
//#include <HTTPClient.h>
//#include <WiFi.h>
//
////HOLD BOOT BUTTON DOWN DURING UPLOAD
////SET MODULE TO ESP32-WROOM DA MODULE
////CONNECT LAPTOP TO MOBILE HOTSPOT ASWELL SO ESP CAN FIND IP ADDRESS
//const char* ssid = "Yash";  //Wifi ID
//const char* password = "123456789";  //Password
//
//
//unsigned long lastTime = 0;
//unsigned long timerDelay = 500;
//
//MPU6050 mpu(Wire);
//Madgwick filter;
//
//// MPU6050 calibration values
//int16_t accelX_offset, accelY_offset, accelZ_offset;
//int16_t gyroX_offset, gyroY_offset, gyroZ_offset;
//
//// Constants for time and distance calculations
//unsigned long previousTime;
//float deltaTime;
//float distanceX, distanceY, distanceZ;
//
//// Robot's initial position
//float posX = 0.0;
//float posY = 0.0;
//float posZ = 0.0;
//
////Flags
//String startFlag = "Wait";
//String wallFlag = "left";
//
//
//
//
////Server Names
//String ServerName = "http://172.20.10.7:3001/"; //local
////String ServerName = "http://44.202.132.63:3001";
//String RoverData = ServerName + "RoverData";
//String RoverControl = ServerName + "RoverControl";
//String RoverStart = ServerName + "RoverStart";
//
//void httpRequests(float posX, float posY, float posZ, float roll, float pitch, float yaw) {
//  //  if ((millis() - lastTime) > timerDelay) {
//    
//  //Check WiFi connection status
//  if(WiFi.status()== WL_CONNECTED){
//    WiFiClient client;
//    HTTPClient http;
//
//    //POST Rover Data
//    Serial.println("Sending Rover Data");
//    http.begin(client, RoverData);
//    http.addHeader("Content-Type", "application/json");
//    int httpResponseCodeP = http.POST("{\"x\":\"" + String(posX) + "\",\"y\":\"" + String(posY) +  "\",\"z\":\"" + String(posZ) + "\",\"Roll\":\"" + String(roll) +  "\",\"Pitch\":\"" + String(pitch) +  "\",\"Yaw\":\"" + String(yaw) + "\"}");
//    Serial.print("HTTP POST Response code: " + String(httpResponseCodeP)+ "\n");
//    http.end();
//
//    //GET Rover Control Instructions
//    Serial.println("Getting Rover Instructions");
//    http.begin(client, RoverControl);
//    int httpResponseCodeG = http.GET();
//    String payload = http.getString();
//    Serial.print("HTTP GET Response code: " + String(httpResponseCodeG) + "\n");
//    Serial.print("HTTP GET Response: " + payload + "\n");
//    wallFlag = payload;
//    http.end();
//
//  }
//
//  else {
//    Serial.println("WiFi Disconnected - Cannot Send Position");
//  }
//
//  //   lastTime = millis();
//  //  }
//}
//
//void RoverStartRequest() {
//  WiFiClient client;
//  HTTPClient http;
//
//  while (startFlag == "Wait") {
//  Serial.println("Getting Rover Start Flag");
//  http.begin(client, RoverStart);
//  int httpResponseCodeStart = http.GET();
//  String payload = http.getString();
//  Serial.print("HTTP GET Response code: " + String(httpResponseCodeStart) + "\n");
//  Serial.print("HTTP GET Response: " + payload + "\n");
//  startFlag = payload;
//  http.end();
//  }
//  Serial.println("Rover Starting");
//}
//
//void WIFIconnect() {
//  WiFi.begin(ssid, password);
//  Serial.println("Connecting");
//  while(WiFi.status() != WL_CONNECTED) {
//    delay(500);
//    Serial.print(".");
//  }
//  Serial.println("");
//  Serial.print("Connected to WiFi network with IP Address: ");
//  Serial.println(WiFi.localIP());
//
//  Serial.println("Timer set to 1 second (timerDelay variable), it will take 1 seconds before publishing the first reading.");
//  delay(1000);
//}
//
//void setup() {
//  // Rover Setup **********************************************************************************************************************
//  Wire.begin();
//  Serial.begin(115200);
//
//  mpu.begin();
//  mpu.calcOffsets();
//
//  filter.begin(10000); // Set filter update rate (in Hz), adjust as needed
//
//  previousTime = micros();
//
//  WIFIconnect();
//  RoverStartRequest();
//
//}
//
//void loop() {
// mpu.update();
//
// // Calculate delta time
// unsigned long currentTime = micros();
// deltaTime = (currentTime - previousTime) / 1000000.0; // Convert to seconds
// previousTime = currentTime;
//
// // Calculate distance traveled in each axis using dead reckoning
// distanceX = (mpu.getGyroX() * deltaTime) + (0.5 * mpu.getAccX() * deltaTime * deltaTime);
// distanceY = (mpu.getGyroY() * deltaTime) + (0.5 * mpu.getAccY() * deltaTime * deltaTime);
// distanceZ = (mpu.getGyroZ() * deltaTime) + (0.5 * mpu.getAccZ() * deltaTime * deltaTime);
//
// // Update robot's position
// posX += distanceX;
// posY += distanceY;
// posZ += distanceZ;
//
// // Update Madgwick filter with sensor data
// filter.updateIMU(mpu.getGyroX(), mpu.getGyroY(), mpu.getGyroZ(), mpu.getAccX(), mpu.getAccY(), mpu.getAccZ());
//
// // Get filtered orientation angles
// float roll = filter.getRoll();
// float pitch = filter.getPitch();
// float yaw = filter.getYaw();
//
// // Print position and orientation values
// Serial.print("Position X: ");
// Serial.print(posX);
// Serial.print("\tY: ");
// Serial.print(posY);
// Serial.print("\tZ: ");
// Serial.println(posZ);
//
// Serial.print("Roll: ");
// Serial.print(roll);
// Serial.print("\tPitch: ");
// Serial.print(pitch);
// Serial.print("\tYaw: ");
// Serial.println(yaw);
//
// httpRequests(posX, posY, posZ, roll, pitch, yaw);
//}
