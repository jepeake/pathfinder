

 

// Basic demo for accelerometer readings from Adafruit MPU6050

 

#include <Adafruit_MPU6050.h>

#include <Adafruit_Sensor.h>

#include <Wire.h>

#include "math.h"

 

#define kp 40

#define kd 0.05

#define ki 40

#define targetangle 0

 

#define dirPin 4

#define stepPin 14

#define stepsPerRevolution 200

#define dirPin2 2

#define stepPin2 15

 

Adafruit_MPU6050 mpu;

float gyroangle, finangle, accangle, currtime, looptime, prevtime=0, error_angle, prevangle=0;

float  error_sum=0;

float motorPower;

 

void setMotors(int leftMotorSpeed, int rightMotorSpeed) {

  if(leftMotorSpeed >= 0) {

    analogWrite(stepPin, leftMotorSpeed);

    digitalWrite(dirPin, LOW);

  }

  else {

    analogWrite(stepPin, 255 + leftMotorSpeed);

    digitalWrite(dirPin, HIGH);

  }

  if(rightMotorSpeed >= 0) {

    analogWrite(stepPin2, rightMotorSpeed);

    digitalWrite(dirPin2, LOW);

  }

  else {

    analogWrite(stepPin2, 255 + rightMotorSpeed);

    digitalWrite(dirPin2, HIGH);

  }

}

 

void setup(void) {

  Serial.begin(115200);

  while (!Serial)

    delay(10); // will pause Zero, Leonardo, etc until serial console opens

 

  Serial.println("Adafruit MPU6050 test!");

 

  // Try to initialize!

  if (!mpu.begin()) {

    Serial.println("Failed to find MPU6050 chip");

    while (1) {

      delay(10);

    }

  }

  Serial.println("MPU6050 Found!");

 

  mpu.setAccelerometerRange(MPU6050_RANGE_8_G);

  Serial.print("Accelerometer range set to: ");

  switch (mpu.getAccelerometerRange()) {

  case MPU6050_RANGE_2_G:

    Serial.println("+-2G");

    break;

  case MPU6050_RANGE_4_G:

    Serial.println("+-4G");

    break;

  case MPU6050_RANGE_8_G:

    Serial.println("+-8G");

    break;

  case MPU6050_RANGE_16_G:

    Serial.println("+-16G");

    break;

  }

  mpu.setGyroRange(MPU6050_RANGE_500_DEG);

  Serial.print("Gyro range set to: ");

  switch (mpu.getGyroRange()) {

  case MPU6050_RANGE_250_DEG:

    Serial.println("+- 250 deg/s");

    break;

  case MPU6050_RANGE_500_DEG:

    Serial.println("+- 500 deg/s");

    break;

  case MPU6050_RANGE_1000_DEG:

    Serial.println("+- 1000 deg/s");

    break;

  case MPU6050_RANGE_2000_DEG:

    Serial.println("+- 2000 deg/s");

    break;

  }

 

  mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);

  Serial.print("Filter bandwidth set to: ");

  switch (mpu.getFilterBandwidth()) {

  case MPU6050_BAND_260_HZ:

    Serial.println("260 Hz");

    break;

  case MPU6050_BAND_184_HZ:

    Serial.println("184 Hz");

    break;

  case MPU6050_BAND_94_HZ:

    Serial.println("94 Hz");

    break;

  case MPU6050_BAND_44_HZ:

    Serial.println("44 Hz");

    break;

  case MPU6050_BAND_21_HZ:

    Serial.println("21 Hz");

    break;

  case MPU6050_BAND_10_HZ:

    Serial.println("10 Hz");

    break;

  case MPU6050_BAND_5_HZ:

    Serial.println("5 Hz");

    break;

  }

 

  Serial.println("");

  delay(100);

    pinMode(stepPin, OUTPUT);

  pinMode(dirPin, OUTPUT);

  pinMode(stepPin2, OUTPUT);

  pinMode(dirPin2, OUTPUT);

}

 

void loop() {

 

  /* Get new sensor events with the readings */

  sensors_event_t a, g, temp;

  mpu.getEvent(&a, &g, &temp);

 

  /* Print out the values */

  Serial.print("Acceleration X: ");

  Serial.print(a.acceleration.x);

  Serial.print(", Y: ");

  Serial.print(a.acceleration.y);

  Serial.print(", Z: ");

  Serial.print(a.acceleration.z);

  Serial.println(" m/s^2");

 

  Serial.print("Rotation X: ");

  Serial.print(g.gyro.x);

  Serial.print(", Y: ");

  Serial.print(g.gyro.y);

  Serial.print(", Z: ");

  Serial.print(g.gyro.z);

  Serial.println(" rad/s");

 

  Serial.print("Temperature: ");

  Serial.print(temp.temperature);

  Serial.println(" degC");

 

  Serial.println("");

 

 

  currtime = millis();

  looptime = currtime - prevtime;

  prevtime = currtime;

 

  gyroangle = (g.gyro.x*looptime)/1000;

  Serial.print("gyroscope angle:");

  Serial.print(gyroangle);

 

 

  accangle = atan(a.acceleration.x/a.acceleration.z);

Serial.print("acceleration angle:");

  Serial.print(accangle);

 

  finangle = 0.98 *(finangle+gyroangle*looptime) + 0.02*accangle;

  Serial.print("precise angle");

  Serial.print(finangle);

 

  delay(500);

 

  error_angle = finangle - targetangle;

  error_sum = error_sum + error_angle;

  error_sum = constrain(error_sum, -300, 300);

 

  motorPower = kp*(error_angle) + ki*(error_sum)*looptime - kd*(finangle-prevangle)/looptime;

  prevangle = finangle;

 

    motorPower = constrain(motorPower, -255, 255);

  setMotors(motorPower, motorPower);










}
