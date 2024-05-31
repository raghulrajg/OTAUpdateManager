#include "miniProject.h"

WiFiClient client;
WiFiClient httpclient;
HTTPClient http;
PubSubClient miniProject::mqttclient(client);

void update_started()
{
  miniProject *instance = new miniProject;
#ifdef WM_DEBUG_LEVEL
  instance->DEBUG_WM(F("CALLBACK:  HTTP update process started"));
#endif
}

void update_finished()
{
  miniProject *instance = new miniProject;
#ifdef WM_DEBUG_LEVEL
  instance->DEBUG_WM(F("CALLBACK:  HTTP update process finished"));
#endif
  ESP.restart();
}

void update_progress(int Cur, int Total)
{
  miniProject *instance = new miniProject;
  digitalWrite(2, HIGH);
#ifdef WM_DEBUG_LEVEL
  instance->DEBUG_WM(F("CALLBACK:  HTTP update process and bytes"), String(Cur) + " " + String(Total));
#endif
  miniProject::mqtt(Cur, Total);
}

void update_error(int err)
{
  miniProject *instance = new miniProject;
#ifdef WM_DEBUG_LEVEL
  instance->DEBUG_WM(F("CALLBACK:  HTTP update fatal error code "), err);
#endif
}

void callback(char *topic, byte *payload, unsigned int length)
{
  miniProject *instance = new miniProject;
  char msg[length + 1];
  memcpy(msg, payload, length);
  msg[length] = '\0';

  StaticJsonDocument<200> doc;
  DeserializationError error = deserializeJson(doc, msg);
  if (error)
  {
#ifdef WM_DEBUG_LEVEL
    instance->DEBUG_WM(F("parsing failed: "), error.c_str());
#endif
  }
  miniProject::_StatusCode = doc["status"];
  //	Serial.print("inside the callback: ");
  //	Serial.println(miniProject::_StatusCode);
}

void miniProject::mqtt(int cur = 0, int total = 0)
{
  StaticJsonDocument<256> doc;
  doc["deviceID"] = String(uniqueId);
  doc["tot"] = total;
  doc["cur"] = cur;
  doc["user"] = _User;
  doc["token"] = _Token;
  char out[200];
  int b = serializeJson(doc, out);
  String topic = "state/" + _User;
  if (mqttclient.publish(topic.c_str(), out))
  {
     //Serial.print("message is send! ");
     //Serial.println(out);
    digitalWrite(2, LOW);
  }
  else
  {
    ESP.restart();
  }

  //  String UpdateTopic = "readytoupdate/"+_User+"/"+_Token;
  //  mqttclient.subscribe(UpdateTopic.c_str());

  mqttclient.loop();
}

void miniProject::wifimanagerConn(char const *apName, char const *apPassword, miniProject *instance)
{
  WiFiManager wm;
  // Serial.begin(115200);
  bool res;
  res = wm.autoConnect(apName, apPassword);
  if (!res)
  {
#ifdef WM_DEBUG_LEVEL
    instance->DEBUG_WM(DEBUG_ERROR, F("Failed to connect"));
#endif
    // ESP.restart();
  }
  else
  {
    // if you get here you have connected to the WiFi
#ifdef WM_DEBUG_LEVEL
    instance->DEBUG_WM(DEBUG_NOTIFY, F("connected...yeey :)"));
#endif
    digitalWrite(2, LOW);
  }
}

void miniProject::OTAUpdate()
{
  WiFiClient client1;
  miniProject *instance = new miniProject;

  miniProject::mqtt();

  if (_StatusCode == Update)
  {
#ifdef WM_DEBUG_LEVEL
    instance->DEBUG_WM(DEBUG_NOTIFY, F("Start Updating Firmware"));
#endif
    httpUpdate.onStart(update_started);
    httpUpdate.onEnd(update_finished);
    httpUpdate.onProgress(update_progress);
    httpUpdate.onError(update_error);

    t_httpUpdate_return ret = httpUpdate.update(client1, host);

    switch (ret)
    {
    case HTTP_UPDATE_FAILED:
#ifdef WM_DEBUG_LEVEL
      instance->DEBUG_WM(DEBUG_ERROR, F("HTTP_UPDATE_FAILED Error: "), (String(httpUpdate.getLastError()) + "  " + String(httpUpdate.getLastErrorString())));
#endif
      break;

    case HTTP_UPDATE_NO_UPDATES:
#ifdef WM_DEBUG_LEVEL
      instance->DEBUG_WM(DEBUG_NOTIFY, F("HTTP_UPDATE_NO_UPDATES"));
#endif
      break;

    case HTTP_UPDATE_OK:
#ifdef WM_DEBUG_LEVEL
      instance->DEBUG_WM(DEBUG_NOTIFY, F("HTTP_UPDATE_OK"));
#endif
      break;
    }
  }
  else if (_StatusCode == user_token_invalid)
  {
#ifdef WM_DEBUG_LEVEL
    instance->DEBUG_WM(DEBUG_ERROR, F("invalid User or Token"));
#endif
  }

  if (WiFi.status() != WL_CONNECTED)
  {
    delay(100);
#ifdef WM_DEBUG_LEVEL
    instance->DEBUG_WM(DEBUG_NOTIFY, F("ESP32 restart"));
#endif
    ESP.restart();
  }
}

// constructor
miniProject::miniProject()
{
  return;
}
miniProject::miniProject(const String &user, const String &token) : miniProject(user, token, ApnOn, "Mini project", "raghulrajg")
{
  return;
}

miniProject::miniProject(const String &user, const String &token, const int Status, char const *ssid, char const *Password)
{

  // Serial.begin(115200);

  _User = user;
  _Token = token;

  //	char const *apName = "Mini project";
  //	char const *apPassword = "raghulrajg";
  deviceId();
  host = _Server + "?user=" + _User + "&token=" + _Token + "&deviceid=" + String(uniqueId);
  pinMode(2, OUTPUT);
  digitalWrite(2, HIGH);
  if (Status == ApnOn)
  {
// DEBUG_PRINTLN(Gr_INFO,"APN WiFi Networking");
#ifdef WM_DEBUG_LEVEL
    DEBUG_WM(DEBUG_NOTIFY, F("APN WiFi Network Start"));
#endif
    wifimanagerConn(ssid, Password, this);
  }
  else if (Status == WiFiOn)
  {
#ifdef WM_DEBUG_LEVEL
    DEBUG_WM(DEBUG_NOTIFY, F("Connecting to WiFi Networking"));
    DEBUG_WM(DEBUG_NOTIFY, F("Connecting to "), String(ssid));
#endif
    // DEBUG_PRINTLN(Gr_INFO,"Connecting to WiFi Networking");
    // DEBUG_PRINTLN(Gr_INFO,"Connecting to "+String(ssid));
    WiFi.begin(ssid, Password);
    while (WiFi.status() != WL_CONNECTED)
    {
      delay(500);
#ifdef WM_DEBUG_LEVEL
      DEBUG_WM(DEBUG_NOTIFY, F("."));
#endif
    }
  }
  else
  {
    char const *apName = "Mini project";
    char const *apPassword = "raghulrajg";
#ifdef WM_DEBUG_LEVEL
    DEBUG_WM(DEBUG_NOTIFY, F("APN WiFi Network Start"));
#endif
    wifimanagerConn(apName, apPassword, this);
  }
  mqttclient.setServer(mqtt_server.c_str(), mqtt_port);
  mqttclient.setCallback(callback);
  while (!mqttclient.connected())
  {
    if (mqttclient.connect(uniqueId, mqtt_username.c_str(), mqtt_password.c_str()))
    {
#ifdef WM_DEBUG_LEVEL
      DEBUG_WM(DEBUG_NOTIFY, F("Server is Connected"));
#endif
    }
    else
    {
#ifdef WM_DEBUG_LEVEL
      DEBUG_WM(DEBUG_ERROR, F("failed with state "), mqttclient.state());
#endif
      delay(2000);
    }
  }
  String topic = "readytoupdate/" + _User + "/" + _Token;
  mqttclient.subscribe(topic.c_str());
  xTaskCreatePinnedToCore(
      loop2,
      "loop2",
      8000,
      NULL,
      0,
      NULL,
      0);
}

char miniProject::uniqueId[24] = "";
String miniProject::jsonBuffer = "";
String miniProject::_User = "";
String miniProject::_Token = "";
String miniProject::host = "";
const String miniProject::_Server = "http://firmware.serveo.net/download";
const String miniProject::mqtt_server = "serveo.net";
const int miniProject::mqtt_port = 2512;
const String miniProject::mqtt_username = "raghulrajg";
const String miniProject::mqtt_password = "Gr2_nemam";
int miniProject::_StatusCode = 0;

String miniProject::httpGETRequest(miniProject *instance, const char *serverName)
{
  http.begin(httpclient, serverName);
  int httpResponseCode = http.GET();
  String payload = "{}";
  if (httpResponseCode > 0)
  {
    payload = http.getString();
  }
  else
  {
#ifdef WM_DEBUG_LEVEL
    instance->DEBUG_WM(DEBUG_ERROR, F("Error code: "), httpResponseCode);
#endif
  }
  http.end();

  return payload;
}

void miniProject::loop2(void *pvParameters)
{
  // miniProject *insta = static_cast<miniProject *>(pvParameters);
  while (1)
  {
    miniProject::OTAUpdate();
  }
}

// DEBUG
// @todo fix DEBUG_WM(0,0);
template <typename Generic>
void miniProject::DEBUG_WM(Generic text)
{
  DEBUG_WM(DEBUG_NOTIFY, text, "");
}

template <typename Generic>
void miniProject::DEBUG_WM(wm_debuglevel_t level, Generic text)
{
  if (_debugLevel >= level)
    DEBUG_WM(level, text, "");
}

template <typename Generic, typename Genericb>
void miniProject::DEBUG_WM(Generic text, Genericb textb)
{
  DEBUG_WM(DEBUG_NOTIFY, text, textb);
}

template <typename Generic, typename Genericb>
void miniProject::DEBUG_WM(wm_debuglevel_t level, Generic text, Genericb textb)
{
  if (!_debug || _debugLevel < level)
    return;

  if (_debugLevel > DEBUG_MAX)
  {
#ifdef ESP8266
// uint32_t free;
// uint16_t max;
// uint8_t frag;
// ESP.getHeapStats(&free, &max, &frag);// @todo Does not exist in 2.3.0
// _debugPort.printf("[MEM] free: %5d | max: %5d | frag: %3d%% \n", free, max, frag);
#elif defined ESP32
    // total_free_bytes;      ///<  Total free bytes in the heap. Equivalent to multi_free_heap_size().
    // total_allocated_bytes; ///<  Total bytes allocated to data in the heap.
    // largest_free_block;    ///<  Size of largest free block in the heap. This is the largest malloc-able size.
    // minimum_free_bytes;    ///<  Lifetime minimum free heap size. Equivalent to multi_minimum_free_heap_size().
    // allocated_blocks;      ///<  Number of (variable size) blocks allocated in the heap.
    // free_blocks;           ///<  Number of (variable size) free blocks in the heap.
    // total_blocks;          ///<  Total number of (variable size) blocks in the heap.
    multi_heap_info_t info;
    heap_caps_get_info(&info, MALLOC_CAP_INTERNAL);
    uint32_t free = info.total_free_bytes;
    uint16_t max = info.largest_free_block;
    uint8_t frag = 100 - (max * 100) / free;
    _debugPort.printf("[MEM] free: %5d | max: %5d | frag: %3d%% \n", free, max, frag);
#endif
  }

  _debugPort.print(_debugPrefix);
  if (_debugLevel >= debugLvlShow)
    _debugPort.print("[" + (String)level + "] ");
  _debugPort.print(text);
  if (textb)
  {
    _debugPort.print(" ");
    _debugPort.print(textb);
  }
  _debugPort.println();
}

void miniProject::deviceId(void)
{
  uint8_t mac[6];
  esp_read_mac(mac, ESP_MAC_WIFI_STA);

  uint64_t chipId = ESP.getEfuseMac();

  snprintf(uniqueId, sizeof(uniqueId), "%04X%08X%02X%02X%02X%02X",
           (uint16_t)(chipId >> 32),
           (uint32_t)chipId,
           mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
}
