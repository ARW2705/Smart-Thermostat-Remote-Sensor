/*===========================
      SUPPORT FUNCTIONS     
===========================*/

/*
 * Get temperature and humidity from DHT sensor
 * 
 * params: none
 * 
 * return: none
 */
void getSensorData() {
  float t = dht.readTemperature(true);
  float h = dht.readHumidity();
  Serial.println("---------------------------");
  Serial.println("Raw sensor data:");
  Serial.print("Temperature: ");
  Serial.print(t);
  Serial.println(" F");
  Serial.print("Humidity:    ");
  Serial.print(h);
  Serial.println(" %");
  Serial.println("---------------------------\n");
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
 * return: bool
 * - true if response from post request is successful
 * 
 * example:
 * {
 *   "temperature": 70.00,
 *   "humidity": 50.00,
 *   "id": 1,
 *   "deviceId": 12
 * }
 */
bool postSensorData() {
  if (WiFi.status() == WL_CONNECTED) {
    char data[100];
    char _t[6];
    char _h[6];
    char _id[2];
    char _deviceId[5];
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
    Serial.println("---------------------------");
    Serial.println(data);
    Serial.println("---------------------------\n");

    http.begin(SERVER_ADDR, HTTP_PORT, "/sensor/update");
    http.addHeader("content-type", "application/json");
    int statusCode = http.POST(data);
    String response = http.getString();
    http.end();

    if (statusCode == 200) {
      Serial.println("Data POST successful");
      int delimiter = response.lastIndexOf(":");
      if (sys.id == -1 && delimiter != -1) {
        sys.selectedDelay = response.substring(0, delimiter) == "inactive" ? LONG_REPORT_DELAY: RAPID_REPORT_DELAY;
        int _id = response.substring(delimiter + 1).toInt();
        if (_id) {
          sys.id = _id;
        } else {
          // TODO post error to server
          return false;
        }
      } else if (sys.id != -1 && delimiter == -1) {
        sys.selectedDelay = response == "inactive" ? LONG_REPORT_DELAY: RAPID_REPORT_DELAY;
      } else {
        // TODO post error to server
        return false;
      }
      return true;
    } else {
      return false;
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
    Serial.println("---------------------------");
    Serial.println(data);
    Serial.println("---------------------------\n");

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

