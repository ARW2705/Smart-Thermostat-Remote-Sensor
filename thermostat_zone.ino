/*======================================
 
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
const int HTTP_PORT = Authorization::SERVER_PORT;
const int DEVICE_ID = 3822; // change for each device
const int DHT_PIN = 2;
const int DHT_TYPE = 22;
const int DHT_MAX = 120;
const int SENSOR_DELAY = 5 * 1000;
const int LONG_REPORT_DELAY = 2 * 60 * 1000;
const int RAPID_REPORT_DELAY = 10 * 1000;
const int INITIALIZE_T_H = -99;

/* END SENSOR CONSTANT DEFINITIONS */


/*========== Global Variable Definitions ==========*/
bool DEBUG = true;

/* END GLOBAL VARIABLE DEFINITIONS */


/*========== Structure Definitions ==========*/
struct System {
  int id = -1; // assigned by thermostat, not dependent on device id
  bool isRapid = false; // true if this zone should be set to rapid tranmission
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
bool postSensorData();
void postRapidConfirmation();
bool isDHTPlausible(float, float);

/* END FUNCTION DEFINITIONS */

/* END DEFINITIONS */


void setup() {
  Serial.begin(9600);
  if (!DEBUG) Serial.end();
  Serial.setDebugOutput(DEBUG);
  Serial.println();
  delay(10);

  
  WiFi.mode(WIFI_STA);
  WiFi.setSleepMode(WIFI_NONE_SLEEP);

  Serial.print("Connecting to ");
  Serial.print(ROUTER_SSID);
  Serial.println("...\n");
  WiFi.begin(ROUTER_SSID, ROUTER_PASS);

  int i = 0;
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(++i);
    Serial.print(' ');
  }
  Serial.print("Connected!");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  Serial.println();
  delay(10);

  Serial.println("DHT sensor activated\n");
  dht.begin();

  timer.sensor = millis();
  timer.report = millis();

  sys.selectedDelay = RAPID_REPORT_DELAY;

  Serial.println("Setup complete");
  Serial.println("===========================================\n");
  delay(100);
}

void loop() {
  if (millis() - timer.sensor > SENSOR_DELAY) {
    getSensorData();
    timer.sensor = millis();
  }

  if (millis() - timer.report > sys.selectedDelay) {
    if (isDHTPlausible(sys.temperature, sys.humidity)) {
      bool success = postSensorData();
      if (success) {
        postRapidConfirmation();
      }
      timer.report = millis();
    } else {
      timer.sensor = millis() - SENSOR_DELAY;
      timer.report = millis() - sys.selectedDelay + 1000;
    }
  }

  delay(100);
}
