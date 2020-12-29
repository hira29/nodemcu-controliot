#include <ArduinoJson.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>

#define trig1 D1
#define echo1 D5
#define trig2 D6
#define echo2 D7
#define LEDL D8
#define LEDR D2

const char* ssid = "NETWORK";
const char* password = "dzakydio21";

String serverName = "http://api.control.fridayio.me/";

// the following variables are unsigned longs because the time, measured in
// milliseconds, will quickly become a bigger number than can be stored in an int.
unsigned long lastTime = 0;
unsigned long lastTimeSensor = 0;
unsigned long timerDelay = 1000;
unsigned long timerDelaySensor = 3000;
// Timer set to 10 minutes (600000)
//unsigned long timerDelay = 600000;
// Set timer to 5 seconds (5000)

float defuz, led, tmp;
float rule[3][3];
float jarak_kanan[3], jarak_kiri[3];
int limit[] = {6, 8, 12, 14, 20};
long duration;
int distanceL, distanceR;
int distance;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  
  WiFi.begin(ssid, password); 
  Serial.println("");
 
  pinMode(LEDL,OUTPUT); 
  pinMode(LEDR,OUTPUT);
  digitalWrite(LEDL,LOW);
  digitalWrite(LEDR,LOW);
  pinMode(trig1, OUTPUT); // Sets the trigPin as an Output
  pinMode(echo1, INPUT); // Sets the echoPin as an Input
  pinMode(trig2, OUTPUT); // Sets the trigPin as an Output
  pinMode(echo2, INPUT); // Sets the echoPin as an Input
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
 
  //If connection successful show IP address in serial monitor
  Serial.println("");
  Serial.print("[Wifi] Connected to ");
  Serial.println(ssid);
  Serial.print("[Wifi] IP address: ");
  Serial.println(WiFi.localIP());
  Serial.println("===============");
  Serial.println("");
}

void loop() {
  // put your main code here, to run repeatedly:
  //Send an HTTP POST request every 10 minutes
  if ((millis() - lastTime) > timerDelay) {
    //Check WiFi connection status
    if(WiFi.status()== WL_CONNECTED){
      Serial.println("[Led State]");
      ledReadL();
      ledReadR();
      Serial.println("===============");
      Serial.println("");
    }
    else {
      Serial.println("[Wifi] WiFi Disconnected");
      Serial.println("===============");
      Serial.println("");
    }
    lastTime = millis();
  }

  if ((millis() - lastTimeSensor) > timerDelaySensor) {
    //Check WiFi connection status
    if(WiFi.status()== WL_CONNECTED){
      Serial.println("[Sense State]");
      senseSend();
      Serial.println("===============");
      Serial.println("");
    }
    else {
      Serial.println("[Wifi] WiFi Disconnected");
      Serial.println("===============");
      Serial.println("");
    }
    lastTimeSensor = millis();
  }
}

void senseSend() {
  
  distanceR = calculateDistanceR();
  delay(500);
  distanceL = calculateDistanceL();


  fuzzy_jarak(distanceR, jarak_kanan);
  fuzzy_jarak(distanceL, jarak_kiri);
  Serial.print("[Sensor]Distance L : ");
  Serial.println(distanceL);
  Serial.print("[Sensor]Distance R : ");
  Serial.println(distanceR);
  defuzzy();

  String serverPathL = serverName + "sensor/L/" + String(distanceL);
  String serverPathR = serverName + "sensor/R/" + String(distanceR);
  
  HTTPClient httpL;
  // Your Domain name with URL path or IP address with path
  httpL.begin(serverPathL.c_str());
  
  // Send HTTP GET request
  int httpLResponseCode = httpL.GET();
  
  if (httpLResponseCode>0) {
    Serial.print("[Data L Sent] HTTP Response code: ");
    Serial.println(httpLResponseCode);
  }
  else {
    Serial.print("[Data L not Sent] Error code: ");
    Serial.println(httpLResponseCode);
  }
  // Free resources
  httpL.end();

  HTTPClient httpR;
  // Your Domain name with URL path or IP address with path
  httpR.begin(serverPathR.c_str());
  
  // Send HTTP GET request
  int httpRResponseCode = httpR.GET();
  
  if (httpRResponseCode>0) {
    Serial.print("[Data R Sent] HTTP Response code: ");
    Serial.println(httpRResponseCode);
  }
  else {
    Serial.print("[Data R not Sent] Error code: ");
    Serial.println(httpRResponseCode);
  }
  // Free resources
  httpR.end();
}

void ledON(String num) {
  String serverPath = serverName + "on/" + num;
  HTTPClient http;
  // Your Domain name with URL path or IP address with path
  http.begin(serverPath.c_str());
  
  // Send HTTP GET request
  int httpResponseCode = http.GET();
  
  if (httpResponseCode>0) {
    Serial.print("["+num+"]");
    Serial.print("[ON]HTTP Response code: ");
    Serial.println(httpResponseCode);
  }
  else {
    Serial.print("["+num+"]");
    Serial.print("[ON]Error code: ");
    Serial.println(httpResponseCode);
  }
  // Free resources
  http.end();
}

void ledOFF(String num) {
  String serverPath = serverName + "off/" + num;
  HTTPClient http;
  // Your Domain name with URL path or IP address with path
  http.begin(serverPath.c_str());
  
  // Send HTTP GET request
  int httpResponseCode = http.GET();
  
  if (httpResponseCode>0) {
    Serial.print("["+num+"]");
    Serial.print("[OFF]HTTP Response code: ");
    Serial.println(httpResponseCode);
  }
  else {
    Serial.print("["+num+"]");
    Serial.print("[OFF]Error code: ");
    Serial.println(httpResponseCode);
  }
  // Free resources
  http.end();
}

void ledReadL(){
  HTTPClient http;
  String serverPath = serverName + "status/1";
  
  // Your Domain name with URL path or IP address with path
  http.begin(serverPath.c_str());
  
  // Send HTTP GET request
  int httpResponseCode = http.GET();
  
  if (httpResponseCode>0) {
    Serial.print("[LED L STATE] HTTP Response code: ");
    Serial.println(httpResponseCode);
    String payload = http.getString();
    
    const size_t bufferSize = JSON_OBJECT_SIZE(3);      
    DynamicJsonBuffer jsonBuffer(bufferSize);
    JsonObject& root = jsonBuffer.parseObject(payload);
    if (!root.success()) {
      Serial.println("[LED L STATE] JSON parsing failed!");
    }
    if (root["data"]){
      Serial.println("[LED L STATE] true");
      digitalWrite(LEDL,HIGH); //LED on
    } else {
      Serial.println("[LED L STATE] false");
      digitalWrite(LEDL,LOW); //LED off
    }
    Serial.println(payload);
  }
  else {
    Serial.print("[LED L STATE] Error code: ");
    Serial.println(httpResponseCode);
  }
  // Free resources
  http.end();
}

void ledReadR(){
  //Send an HTTP POST request every 10 minutes
  HTTPClient http;
  String serverPath = serverName + "status/2";
  
  // Your Domain name with URL path or IP address with path
  http.begin(serverPath.c_str());
  
  // Send HTTP GET request
  int httpResponseCode = http.GET();
  
  if (httpResponseCode>0) {
    Serial.print("[LED R STATE] HTTP Response code: ");
    Serial.println(httpResponseCode);
    String payload = http.getString();
    
    const size_t bufferSize = JSON_OBJECT_SIZE(3);      
    DynamicJsonBuffer jsonBuffer(bufferSize);
    JsonObject& root = jsonBuffer.parseObject(payload);
    if (!root.success()) {
      Serial.println("JSON parsing failed!");
    }
    if (root["data"]){
      Serial.println("[LED R STATE] true");
      digitalWrite(LEDR,HIGH); //LED on
    } else {
      Serial.println("[LED R STATE] false");
      digitalWrite(LEDR,LOW); //LED off
    }
    Serial.println(payload);
  }
  else {
    Serial.print("[LED R STATE] Error code: ");
    Serial.println(httpResponseCode);
  }
  // Free resources
  http.end();
}

int calculateDistanceL(){ 
  digitalWrite(trig1, LOW); 
  delayMicroseconds(2);
  // Sets the trigPin on HIGH state for 10 micro seconds
  digitalWrite(trig1, HIGH); 
  delayMicroseconds(10);
  digitalWrite(trig1, LOW);
  duration = pulseIn(echo1, HIGH); // Reads the echoPin, returns the sound wave travel time in microseconds
  distanceL = duration*0.034/2;
  return distanceL;
}

int calculateDistanceR(){ 
  digitalWrite(trig2, LOW); 
  delayMicroseconds(2);
  // Sets the trigPin on HIGH state for 10 micro seconds
  digitalWrite(trig2, HIGH); 
  delayMicroseconds(10);
  digitalWrite(trig2, LOW);
  duration = pulseIn(echo2, HIGH); // Reads the echoPin, returns the sound wave travel time in microseconds
  distanceR = duration*0.034/2;
  return distanceR;
}

void fuzzy_jarak(float detected, float jarak[]) {
  // jarak dekat
  if(detected <= limit[0])
    jarak[0] = 1;
   else if (detected > limit[0] && detected <= limit[1])
    jarak[0] = (limit[1] - detected)/(limit[1] - limit[0]);
   else
    jarak[0] = 0;

   // jarak sedang
   if(detected <= limit[0])
    jarak[1] = 0;
   else if (detected > limit[0] && detected <= 10)
    jarak[1] = (detected - limit[0])/(10 - limit[0]);
   else if (detected > 10 && detected <= limit[3])
    jarak[1] = (limit[3] - detected)/(limit[3] - 10);
   else
    jarak[1] = 0;

   // jarak jauh
   if(detected <= limit[2])
    jarak[2] = 0;
   else if (detected > limit[2] && detected <= limit[3])
    jarak[2] = (detected - limit[2])/(limit[3] - limit[2]);
   else
    jarak[2] = 1;
}

void fuzzy_rule(){
  for (int i = 0; i < 3; i++) {
    for (int j = 0; j < 3; j++) {
      rule[i][j] = min(jarak_kanan[i], jarak_kiri[j]);
    }
  }
}

void defuzzy() {
  // metode sugeno
  /**** VARIABEL OUTPUT ****/
  // TODO: cari parameter hasil untuk menentukan output defuzzy
  float kanan = 1;
  float tengah = 2;
  float kiri = 3;
  /*************************/

  fuzzy_rule();
  led = (rule[0][0] * tengah) + (rule[0][1] * kiri) + (rule[0][2] * kiri)
        + (rule[1][0] * kanan) + (rule[1][1] * tengah) + (rule[1][2] * kiri)
        + (rule[2][0] * kanan) + (rule[2][1] * kanan) + (rule[2][2] * tengah);
  
  defuz = 0;
  for(int i = 0; i < 3; i++)
    for(int j = 0; j < 3; j++)
      defuz += rule[i][j];

  led = led/defuz;
  Serial.print("[Fuzzy] Fuzzy Data : ");
  Serial.println(led);
  if (tmp != led){
    Serial.println("[Fuzzy] Result Change!");
    if (led <= 1.0) {
        Serial.println("[Fuzzy] Result : Right");
        ledOFF("1");
        ledON("2");
    } else if (led <= 2) {
        Serial.println("[Fuzzy] Result : Center");
        ledON("1");
        ledON("2");
    } else {
        Serial.println("[Fuzzy] Result : Left");
        ledON("1");
        ledOFF("2");
    }
  } else {
    Serial.println("[Fuzzy] No Result Change");
  }

  tmp = led;
}
