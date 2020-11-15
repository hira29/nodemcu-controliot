#include <ArduinoJson.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>
#define trig D6
#define echo D7
#define LED1 D1
#define LED2 D2

const char* ssid = "NETWORK";
const char* password = "dzakydio21";

String serverName = "http://api.control.fridayio.me/";

// the following variables are unsigned longs because the time, measured in
// milliseconds, will quickly become a bigger number than can be stored in an int.
unsigned long lastTime = 0;
unsigned long lastTimeSensor = 0;
unsigned long timerDelay = 1000;
unsigned long timerDelaySensor = 20000;
// Timer set to 10 minutes (600000)
//unsigned long timerDelay = 600000;
// Set timer to 5 seconds (5000)


long duration;
int distance;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  
  WiFi.begin(ssid, password); 
  Serial.println("");
 
  pinMode(LED1,OUTPUT); 
  pinMode(LED2,OUTPUT);
  digitalWrite(LED1,LOW);
  digitalWrite(LED2,LOW);
  pinMode(trig, OUTPUT); // Sets the trigPin as an Output
  pinMode(echo, INPUT); // Sets the echoPin as an Input
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
 
  //If connection successful show IP address in serial monitor
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}

void loop() {
  // put your main code here, to run repeatedly:
  //Send an HTTP POST request every 10 minutes
  if ((millis() - lastTime) > timerDelay) {
    //Check WiFi connection status
    if(WiFi.status()== WL_CONNECTED){
      ledRead1();
      ledRead2();
    }
    else {
      Serial.println("WiFi Disconnected");
    }
    lastTime = millis();
  }

  if ((millis() - lastTimeSensor) > timerDelaySensor) {
    //Check WiFi connection status
    if(WiFi.status()== WL_CONNECTED){
      senseSend();
    }
    else {
      Serial.println("WiFi Disconnected");
    }
    lastTimeSensor = millis();
  }
}

void senseSend() {
  HTTPClient http;
  distance = calculateDistance();
  
  String serverPath = serverName + "sensor/" + String(distance);
  
  // Your Domain name with URL path or IP address with path
  http.begin(serverPath.c_str());
  
  // Send HTTP GET request
  int httpResponseCode = http.GET();
  
  if (httpResponseCode>0) {
    Serial.print("Distance : ");
    Serial.println(distance);
    Serial.print("HTTP Response code: ");
    Serial.println(httpResponseCode);
  }
  else {
    Serial.print("Distance : ");
    Serial.println(distance);
    Serial.print("Error code: ");
    Serial.println(httpResponseCode);
  }
  // Free resources
  http.end();
}

void ledRead1(){
  HTTPClient http;
  String serverPath = serverName + "status/1";
  
  // Your Domain name with URL path or IP address with path
  http.begin(serverPath.c_str());
  
  // Send HTTP GET request
  int httpResponseCode = http.GET();
  
  if (httpResponseCode>0) {
    Serial.print("HTTP Response code: ");
    Serial.println(httpResponseCode);
    String payload = http.getString();
    
    const size_t bufferSize = JSON_OBJECT_SIZE(3);      
    DynamicJsonBuffer jsonBuffer(bufferSize);
    JsonObject& root = jsonBuffer.parseObject(payload);
    if (!root.success()) {
      Serial.println("JSON parsing failed!");
    }
    if (root["data"]){
      Serial.println("true");
      digitalWrite(LED1,HIGH); //LED on
    } else {
      Serial.println("false");
      digitalWrite(LED1,LOW); //LED off
    }
    Serial.println(payload);
  }
  else {
    Serial.print("Error code: ");
    Serial.println(httpResponseCode);
  }
  // Free resources
  http.end();
}

void ledRead2(){
  //Send an HTTP POST request every 10 minutes
  HTTPClient http;
  String serverPath = serverName + "status/2";
  
  // Your Domain name with URL path or IP address with path
  http.begin(serverPath.c_str());
  
  // Send HTTP GET request
  int httpResponseCode = http.GET();
  
  if (httpResponseCode>0) {
    Serial.print("HTTP Response code: ");
    Serial.println(httpResponseCode);
    String payload = http.getString();
    
    const size_t bufferSize = JSON_OBJECT_SIZE(3);      
    DynamicJsonBuffer jsonBuffer(bufferSize);
    JsonObject& root = jsonBuffer.parseObject(payload);
    if (!root.success()) {
      Serial.println("JSON parsing failed!");
    }
    if (root["data"]){
      Serial.println("true");
      digitalWrite(LED2,HIGH); //LED on
    } else {
      Serial.println("false");
      digitalWrite(LED2,LOW); //LED off
    }
    Serial.println(payload);
  }
  else {
    Serial.print("Error code: ");
    Serial.println(httpResponseCode);
  }
  // Free resources
  http.end();
}

int calculateDistance(){ 
  digitalWrite(trig, LOW); 
  delayMicroseconds(2);
  // Sets the trigPin on HIGH state for 10 micro seconds
  digitalWrite(trig, HIGH); 
  delayMicroseconds(10);
  digitalWrite(trig, LOW);
  duration = pulseIn(echo, HIGH); // Reads the echoPin, returns the sound wave travel time in microseconds
  distance= duration*0.034/2;
  return distance;
}
