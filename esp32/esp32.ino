#include <WiFi.h>
#include <HTTPClient.h>
#include <stdlib.h>

//HOLD BOOT BUTTON DOWN DURING UPLOAD
//SET MODULE TO ESP32-WROOM DA MODULE
//CONNECT LAPTOP TO MOBILE HOTSPOT ASWELL SO ESP CAN FIND IP ADDRESS
const char* ssid = "gobee"; 
const char* password = "123456789";

//Your Domain name with URL path or IP address with path
String serverName = "http://172.20.10.2:3001/RoverPosition"; 

unsigned long lastTime = 0;
unsigned long timerDelay = 5000;

void setup() {
  Serial.begin(115200); 

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

void loop() {
  if ((millis() - lastTime) > timerDelay) {
    
    //Check WiFi connection status
    if(WiFi.status()== WL_CONNECTED){
      WiFiClient client;
      HTTPClient http;
    
      // Your Domain name with URL path or IP address with path
      http.begin(client, serverName);
      for (int a = 0; a<=400; a = a + 10){
        http.addHeader("Content-Type", "application/json");
        int httpResponseCode = http.POST("{\"x\":\"" + String(a) + "\",\"y\":\"" + String(a) + "\"}");
        Serial.print("HTTP Response code: ");
        Serial.println(httpResponseCode);
        delay(100);
      }
      http.end();
    }
    else {
      Serial.println("WiFi Disconnected");
    }
    lastTime = millis();
  }
}
