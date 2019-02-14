/*======================================
 * 
  Remote Temperature/Humidity Sensor
  
  ESP8266
  
======================================*/

/*========== Library Imports ==========*/
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <DHT.h>
#include "AuthConstants.h"

/* END IMPORTS */


/*========== Sensor CONSTANT Definitions ==========*/
const char* ROUTER_SSID = Authorization::ROUTER_SSID;
const char* ROUTER_PASS = Authorization::ROUTER_PASS;
const char* SERVER_ADDR = Authorization::SERVER_ADDR;
const int HTTP_PORT = 2772;
const int DEVICE_ID = 1;
const int DHT_PIN = 0;
const int DHT_TYPE = 22;
const int DHT_MAX = 120;
const int SENSOR_DELAY = 10 * 1000;
const int LONG_REPORT_DELAY = 2 * 60 * 1000;
const int RAPID_REPORT_DELAY = 10 * 1000;
const int INITIALIZE_T_H = -99;

/* END SENSOR CONSTANT DEFINITIONS */

/*========== Structure Definitions ==========*/
struct System {
  int id = -1;
  bool isRapid = false;
  int selectedDelay;
  float temperature = INITIALIZE_T_H;
  float humidity = INITIALIZE_T_H;
};

struct Timer {
  uint64_t sensor;
  uint64_t report;
};

/* END STRUCTURE DEFINITIONS */


/*========== Object/Structure Initialization ==========*/
DHT dht(DHT_PIN, DHT_TYPE);
HTTPClient http;
System sys;
Timer timer;

/* END OBJECT/STRUCTURE INITIALIZATIONS */

/*========== Function Declarations ==========*/
void getSensorData();
void postSensorData();
void postRapidConfirmation();
bool isDHTPlausible(float, float);

/* END FUNCTION DEFINITIONS */

/* END DEFINITIONS */

void setup() {
  WiFi.mode(WIFI_STA);

  WiFi.begin(ROUTER_SSID, ROUTER_PASS);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
  }

  dht.begin();

  timer.sensor = millis();
  timer.report = millis();

  sys.selectedDelay = RAPID_REPORT_DELAY;

  delay(100);
}

void loop() {
  if (millis() - timer.sensor > SENSOR_DELAY) {
    getSensorData();
    timer.sensor = millis();
  }

  if (millis() - timer.report > sys.selectedDelay) {
    if (isDHTPlausible(sys.temperature, sys.humidity)) {
      postSensorData();
      timer.report = millis();
    } 
  }

  delay(100);
}
