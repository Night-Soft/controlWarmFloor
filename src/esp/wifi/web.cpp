#include "web.h"

#include <ArduinoJson.h>
#include <ESP8266WiFi.h>
#include <ESPAsyncWebServer.h>
#include <FileData.h>
#include <LittleFS.h>  // Include the LittleFS library

#include "../PostponedTask.h"
#include "../TimeRange.h"
#include "../display.h"
#include "../actions.h"

AsyncWebServer server(80);
WiFiHolder* wifiHolder = nullptr;

bool isWebServer = false;

void setupFS() {
  
  if(!LittleFS.begin()){
    Serial.println("An Error has occurred while mounting LittleFS");
    return;
  }
  File file = LittleFS.open("/test_example.txt", "r");

  //File file = LittleFS.open("/index.html", "r");
  if(!file){
    Serial.println("Failed to open file for reading");
    return;
  }
  
  Serial.println("File Content:");
  while(file.available()){
    Serial.write(file.read());
  }
  file.close();
}

void endServer() {
  Serial.println("End web server");
  //display.hideModal();
  Modal modal = {"", "Web server off!"};
  modal.timeShow = 5;
  display.showModal(modal);
  
  isWebServer = false;
  server.end();
  if (wifiHolder != nullptr) delete wifiHolder;
}

unsigned long lastActivity = 0;
void endServerIfNoActivity() {
  static uint32 prev = lastActivity;
  uint32 now = millis();
  
  Serial.printf("check server if no activity: %d\n", now - prev);
  prev = now;
  if (isWebServer == false) {
    Serial.println("Webserver already off!");
    Serial.println("Remove task endServerIfNoActivity!");
    postponedTask.remove(endServerIfNoActivity);
  }

  if (now - lastActivity > 3 * 60 * 1000) {  // 3 minutes
    endServer();
    postponedTask.remove(endServerIfNoActivity);
    Serial.println("Task 'endServerIfNoActivity' should be removed!");
  }
}

auto onToggleRelay = [](AsyncWebServerRequest* request) {
  if (request->hasParam("enable")) {
    if (request->getParam("enable")->value() == "true") {
      postponedTask.setTimeout(0, []() {
        NextAction action;
        action.action = U_ACTIONS::pumpTurnOn;
        executeAction(action);
      });
      request->send(200, "application/json", "{\"status\": true}");
    }
    if (request->getParam("enable")->value() == "false") {
      postponedTask.setTimeout(0, []() {
        NextAction action;
        action.action = U_ACTIONS::pumpTurnOff;
        executeAction(action);
      });
      request->send(200, "application/json", "{\"status\": false}");
    }
    return;
  }
  request->send(200, "application/json", "{\"status\":\"ok\"}");
};

auto onApi = [](AsyncWebServerRequest* request) {
  lastActivity = millis();
  char json[35] = "{\"status\":\"ok\", \"relay\": true }"; 
  if(!pump.isWorking()) {
    //strncpy(&json[23], "false", 5);
    std::copy_n("false", 5, &json[25]);
  }
  request->send(200, "application/json", json);
};

auto onGetTime = [](AsyncWebServerRequest* request) {
  lastActivity = millis();
  Serial.println("getTime");
  
  timeRanges.read();
  printTimeRanges(timeRanges.ranges, timeRanges.size);

  if (timeRanges.size == 0) {
    Serial.println("File is empty!");
    request->send(200, "application/json", "{\"status\": \"File is empty!\"}");
    return;
  }

  const char* message;
  switch (timeRanges.status) {
    case FD_FS_ERR:  // 2 - File system error
      message = "{\"status\": \"File system error!\"}";
      break;

    case FD_FILE_ERR:  // 3 - Error opening file
      message = "{\"status\": \"Error opening file!\"}";
      break;

    case FD_READ:  // 5 - Reading data from file
      if (strlen(timeRanges.json) > 5) {
        message = timeRanges.json;
      } else {
        message = "{\"status\": \"File is empty!\"}";
      }
      break;

    default:
      message = "{\"status\": \"Unknown error while read!\"}";
      break;
  }
  Serial.printf("Read status: %d\n", (int)timeRanges.status);
  request->send(200, "application/json", message);
};

FDstat_t statusSaveTime = FDstat_t::FD_IDLE;

auto onSetTimeRequest =
    [](AsyncWebServerRequest* request) {  // call after onBody
      if (statusSaveTime == FD_WRITE || statusSaveTime == FD_NO_DIF) {
        request->send(200, "application/json", "{\"status\":\"Success save\"}");
      } else {
        request->send(200, "application/json", "{\"status\":\"Error save\"}");
      }
      Serial.printf("Write status: %d\n", (int)statusSaveTime);
    };

// onBody — сюда приходит тело запроса    
auto onSetTimeBody = [](AsyncWebServerRequest* request, uint8_t* data,
                        size_t len, size_t index, size_t total) {
  lastActivity = millis();

  statusSaveTime = timeRanges.write(data, len);
  printTimeRanges(data, len);
};

void displayInitWebServer() {
  Modal modal = {"","Web server loading..."};
  display.showModal(modal);
  display.drawNow();
}

bool initWebServer() {
  displayInitWebServer();
  wifiHolder = new WiFiHolder(); 
  if (!wifiHolder->turnOn()) {
    Serial.println("Error init WebServer");
    Modal modal = {"", "Error init Web server.", "Try again!"};
    modal.timeShow = 5;
    display.showModal(modal);

    delete wifiHolder; 
    return false;
  }

  Serial.println();
  Serial.printf("\n %10s", "Connect to: ");
  Serial.println(WiFi.localIP());
  Serial.println();

  if (!LittleFS.begin()) {
    Serial.println("LittleFS mount failed");
    Serial.println("Error init WebServer");
    delete wifiHolder; // turnOff wifi if no holders
    return false;
  } else {
    Serial.println("LittleFS mounted successfully");
  }

  server.serveStatic("/", LittleFS, "/").setDefaultFile("index.html");

  server.on("/api", HTTP_GET, onApi);
  server.on("/getTime", HTTP_GET, onGetTime);
  server.on("/toggleRelay", HTTP_GET, onToggleRelay);
  server.on("/saveTime", HTTP_POST, onSetTimeRequest,
            NULL,  
            onSetTimeBody);

  server.begin();
  Serial.println("Async server started");

  lastActivity = millis();

  Modal modal;
  sprintf(modal.title, "Setup time!");
  sprintf(modal.str1, "Connect to:");
  sprintf(modal.str2, WiFi.localIP().toString().c_str());
  display.showModal(modal);

  postponedTask.setInterval(30 * 1000, endServerIfNoActivity);  

  isWebServer = true;
  return true;
}