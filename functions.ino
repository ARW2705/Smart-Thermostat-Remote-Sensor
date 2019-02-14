/*===========================
      SUPPORT FUNCTIONS     
===========================*/

void getSensorData() {
  float t = dht.readTemperature(true);
  float h = dht.readHumidity();
  if (isDHTPlausible(t, h)) {
    sys.temperature = t;
    sys.humidity = h;
  } else {
    sys.temperature = INITIALIZE_T_H;
    sys.humidity = INITIALIZE_T_H;
  }
}

/*
 * Post sensor data to thermostat
 * 
 * params: none
 * 
 * return: none
 * 
 * example:
 * {
 *   "temperature": 70.00,
 *   "humidity": 50.00,
 *   "id": 1,
 *   "deviceId": 12
 * }
 */
void postSensorData() {
  if (WiFi.status() == WL_CONNECTED) {
    char data[100];
    char _t[6];
    char _h[6];
    char _id[2];
    char _deviceId[3];
    sprintf(_t, "%.2f", sys.temperature);
    sprintf(_h, "%.2f", sys.humidity);
    sprintf(_id, "%d", sys.id);
    sprintf(_deviceId, "%d", DEVICE_ID);
    strcpy(data, "{\"temperature\": ");
    strcat(data, _t);
    strcat(data, ",\"humidity\": ");
    strcat(data, _h);
    strcat(data, ",\"id\": ");
    strcat(data, _id);
    strcat(data, ",\"deviceId\": ");
    strcat(data, _deviceId);
    strcat(data, ",\"isRapid\": ");
    strcat(data, sys.isRapid ? "true": "false");
    strcat(data, "}");

    http.begin(SERVER_ADDR, HTTP_PORT, "/sensor");
    http.addHeader("content-type", "application/json");
    int statusCode = http.POST(data);
    String response = http.getString();
    http.end();

    if (statusCode == 200) {
      sys.selectedDelay = response == "inactive" ? LONG_REPORT_DELAY: RAPID_REPORT_DELAY;
      postRapidConfirmation();
    }
  }
}

void postRapidConfirmation() {
  if (WiFi.status() == WL_CONNECTED) {
    char data[64];
    char _id[2];
    sprintf(_id, "%d", sys.id);
    strcpy(data, "{\"id\": ");
    strcat(data, _id);
    strcat(data, ",\"isRapid\": ");
    strcat(data, sys.isRapid ? "true": "false");
    strcat(data, "}");

    http.begin(SERVER_ADDR, HTTP_PORT, "/sensor/confirm-rapid");
    http.addHeader("content-type", "application/json");
    http.POST(data);
    http.end();
  }
}

bool isDHTPlausible(float t, float h) {
  return (!isnan(t) && !isnan(h)
          && 0 < t && t < DHT_MAX
          && 0 < h && h < DHT_MAX);
}

