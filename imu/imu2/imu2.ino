#include <Wire.h>
#include <MPU6050_light.h>
#include <AccelStepper.h>
#include <MadgwickAHRS.h>


#define MOTOR1_DIR_PIN 14
#define MOTOR1_STEP_PIN 4
#define MOTOR2_DIR_PIN 15
#define MOTOR2_STEP_PIN 2

AccelStepper stepperLeft(AccelStepper::DRIVER, MOTOR1_STEP_PIN, MOTOR1_DIR_PIN);
AccelStepper stepperRight(AccelStepper::DRIVER, MOTOR2_STEP_PIN, MOTOR2_DIR_PIN);

int16_t accelX_offset, accelY_offset, accelZ_offset;
int16_t gyroX_offset, gyroY_offset, gyroZ_offset;

const double wheelDiameter = 0.064; // in meters
const double stepsPerRevolution = 200.0;
const double wheelBase = 0.17; // in meters, distance between two wheels

MPU6050 imu(Wire);
Madgwick filter;

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

  filter.begin(1000); 

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
  float posX = 0, posY = 0;

  imu.update();
  
  filter.updateIMU(imu.getGyroX(), imu.getGyroY(), imu.getGyroZ(), imu.getAccX(), imu.getAccY(), imu.getAccZ());

  float yaw = (imu.getAngleY());

  long leftSteps = stepperLeft.currentPosition();
  long rightSteps = stepperRight.currentPosition();

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

  fwd();
}
