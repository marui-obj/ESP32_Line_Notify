#include <Arduino.h>
#include <WiFi.h>
#include <LM73.h>
#include "ap_secrets.h"
#include "http_secrets.h"

#include <HTTPClient.h>
#include <WiFiClientSecure.h>

LM73 lm73 = LM73();
WiFiClientSecure client;
HTTPClient http;

String endPoint = "https://notify-api.line.me/api/notify";
String apiKey = SECRET_TOKEN_KEY;





const char* ssid = SECRET_SSID;
const char* password = SECRET_PASS;


void connectWifi(){
  /* Connect Access Point with SSID and password */
  /* If Wifi not connect then polling unitl it connect */
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.println("Connecting");
  while(WiFi.status() != WL_CONNECTED){
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to WiFi network with IP Address: ");
  Serial.println(WiFi.localIP());
  client.setCACert(ca_cert);
}

int httpRequest(float temp, const char* message){
  /* Use HTTP Methods to Requset Line Notify */
  /* This request is in URL encoded format. */
    String payload = "message="+ String(message) +" Temp= " + String(temp) + "°C";

    http.begin(client, endPoint);

    http.addHeader("Content-Type", "application/x-www-form-urlencoded");
    http.addHeader("Authorization", "Bearer " + String(SECRET_TOKEN_KEY));
    
    int status = http.POST(payload);
    http.end();
    return status;
}

const char* stateControl(float temp){
  if(temp >= 30) return "It's so hot today ☀️";
  else if(temp < 30 && temp >= 20) return "It's a beautiful day 😎";
  else if (temp < 20) return "it's cold 🥶";
}

void setup() {
  Serial.begin(9600);
  connectWifi();
  lm73.begin();
}

void loop() {
  static uint32_t last_time;
  if (millis() - last_time > 5000){
    if (WiFi.status() == WL_CONNECTED){
      float val = lm73.getVal(14);
      static const char* temp_message;
      const char* message = stateControl(val);
      if (temp_message != message){
        int response = httpRequest(val, message);
        Serial.println(response);
        if (response == 200){
          temp_message = message;
        }
      }
    }
    last_time = millis();
  }
}