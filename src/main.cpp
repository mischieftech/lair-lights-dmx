#include <Arduino.h>
#include <ESPAsyncWebServer.h>
#include <WiFi.h>
#include "esp_task_wdt.h"

const int writePin = 25;
const int dmxPin = 17;
uint8_t dmxbuffer[513] = {};
int maxChannel = 10;

const char* ssid = "";
const char* password = "";
AsyncWebServer server(80);

void dmxUpdate();
void setValue(int channel, uint8_t value);


void setup() {
  Serial.begin(115200);
  Serial.println("Begin setup");

  // connect to wifi
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi.");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("Connected to WiFi. IP Address:");
  Serial.println(WiFi.localIP());

  // Start serial2 for DMX-512
  Serial2.begin(250000, SERIAL_8N2, -1, dmxPin);
  for (int i = 0; i < 512; i++)
    dmxbuffer[i] = 0;

  // Configure the server
  server.on("/setDMX", HTTP_GET | HTTP_POST, [](AsyncWebServerRequest *request) {
    int params = request->params();
    for (int i = 0; i < params; i++) {
      AsyncWebParameter* p = request->getParam(i);
      int channel = p->name().toInt();
      int value = p->value().toInt();
      setValue(channel, value);
    }

    dmxUpdate();
    request->send(200, "application/json", "{\"status\":\"success\"}");
  });
  server.begin();
    
  Serial.println("Setup complete");
}

void dmxUpdate() {
  // Send break
  Serial1.begin(83333, SERIAL_8N1, -1, dmxPin);
  Serial1.write(0);
  Serial1.flush();
  delay(1);
  Serial1.end();

  // send data
  Serial1.begin(250000, SERIAL_8N2, -1, dmxPin);
  Serial1.write(dmxbuffer, maxChannel + 1);
  Serial1.flush();
  delay(1);
  Serial1.end();
}

void setValue(int channel, uint8_t value) {
  channel = max(1, min(512, channel));
  maxChannel = max(maxChannel, channel);
  dmxbuffer[channel] = value;
}

void loop() {
  dmxUpdate();
  vTaskDelay(100);
}