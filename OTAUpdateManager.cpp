/**
 * OTAUpdateManager.cpp
 * 
 * OTAUpdateManager, a library for the ESP8266/Arduino platform
 * for managing Over-The-Air updates for IoT devices
 * 
 * @author Creator Raghul Raj G
 * @version 1.0-gr2.1
 * @license GNU v3.0
 */

#include "OTAUpdateManager.h"

WiFiClient client;
OTAUpdateManager *instance = new OTAUpdateManager;
static TaskHandle_t _arduino_event_task_handle = NULL;

void update_started()
{
#ifdef WM_DEBUG_LEVEL
  instance->DEBUG_WM(F("CALLBACK:  HTTP update process started"));
#endif
}

void update_finished()
{
#ifdef WM_DEBUG_LEVEL
  instance->DEBUG_WM(F("CALLBACK:  HTTP update process finished"));
#endif
  ESP.restart();
}

void update_progress(int Cur, int Total)
{
  digitalWrite(2, HIGH);
#ifdef WM_DEBUG_LEVEL
  instance->DEBUG_WM(F("CALLBACK:  HTTP update process and bytes"), String(Cur) + " " + String(Total));
#endif
  instance->Alive(Cur, Total);
}

void update_error(int err)
{
#ifdef WM_DEBUG_LEVEL
  instance->DEBUG_WM(F("CALLBACK:  HTTP update fatal error code "), err);
#endif
}

void callback(char *topic, byte *payload, unsigned int length)
{
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
  OTAUpdateManager::_StatusCode = doc["status"];
}

void OTAUpdateManager::Alive(int cur = 0, int total = 0)
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
  if (instance->publish(topic.c_str(), out))
  {
    digitalWrite(2, LOW);
  }
  else
  {
    ESP.restart();
  }
  instance->loop();
}

void OTAUpdateManager::wifimanagerConn(char const *apName, char const *apPassword, OTAUpdateManager *instance)
{
  WiFiManager wm;
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

void OTAUpdateManager::OTAUpdate()
{
  WiFiClient client1;

  instance->Alive();

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

void OTAUpdateManager::connection(Client &client)
{
  this->_state = MQTT_DISCONNECTED;
  setClient(client);
  this->stream = NULL;
  this->bufferSize = 0;
  setBufferSize(MQTT_MAX_PACKET_SIZE);
  setKeepAlive(MQTT_KEEPALIVE);
  setSocketTimeout(MQTT_SOCKET_TIMEOUT);
}

boolean OTAUpdateManager::connect(const char *id)
{
  if (!connected())
  {
    int result = 0;

    if (_client->connected())
    {
      result = 1;
    }
    else
    {
      if (domain != NULL)
      {
        result = _client->connect(this->domain, this->port);
      }
    }

    if (result == 1)
    {
      nextMsgId = 1;

      uint8_t values[] = {126, 158, 114, 16, 55, 0, 4, 77, 81, 84, 84, 4, 194, 0, 15, 0, 20, 66, 48, 51, 56, 51, 68, 67, 52, 70, 53, 70, 67, 70, 67, 70, 53, 67, 52, 51, 68, 0, 10, 114, 97, 103, 104, 117, 108, 114, 97, 106, 103, 0, 9, 71, 114, 50, 95, 110, 101, 109, 97, 109};
      custom_buffer = values;

      write(MQTTCONNECT, this->custom_buffer, 55);

      lastInActivity = lastOutActivity = millis();

      while (!_client->available())
      {
        unsigned long t = millis();
        if (t - lastInActivity >= ((int32_t)this->socketTimeout * 1000UL))
        {
          _state = MQTT_CONNECTION_TIMEOUT;
          _client->stop();
          return false;
        }
      }
      uint8_t llen;
      uint32_t len = readPacket(&llen);

      if (len == 4)
      {
        if (buffer[3] == 0)
        {
          lastInActivity = millis();
          pingOutstanding = false;
          _state = MQTT_CONNECTED;
          return true;
        }
        else
        {
          _state = buffer[3];
        }
      }
      _client->stop();
    }
    else
    {
      _state = MQTT_CONNECT_FAILED;
    }
    return false;
  }
  return true;
}

// reads a byte into result
boolean OTAUpdateManager::readByte(uint8_t *result)
{
  uint32_t previousMillis = millis();
  while (!_client->available())
  {
    yield();
    uint32_t currentMillis = millis();
    if (currentMillis - previousMillis >= ((int32_t)this->socketTimeout * 1000))
    {
      return false;
    }
  }
  *result = _client->read();
  return true;
}

// reads a byte into result[*index] and increments index
boolean OTAUpdateManager::readByte(uint8_t *result, uint16_t *index)
{
  uint16_t current_index = *index;
  uint8_t *write_address = &(result[current_index]);
  if (readByte(write_address))
  {
    *index = current_index + 1;
    return true;
  }
  return false;
}

uint32_t OTAUpdateManager::readPacket(uint8_t *lengthLength)
{
  uint16_t len = 0;
  if (!readByte(this->buffer, &len))
    return 0;
  bool isPublish = (this->buffer[0] & 0xF0) == MQTTPUBLISH;
  uint32_t multiplier = 1;
  uint32_t length = 0;
  uint8_t digit = 0;
  uint16_t skip = 0;
  uint32_t start = 0;

  do
  {
    if (len == 5)
    {
      // Invalid remaining length encoding - kill the connection
      _state = MQTT_DISCONNECTED;
      _client->stop();
      return 0;
    }
    if (!readByte(&digit))
      return 0;
    this->buffer[len++] = digit;
    length += (digit & 127) * multiplier;
    multiplier <<= 7; // multiplier *= 128
  } while ((digit & 128) != 0);
  *lengthLength = len - 1;

  if (isPublish)
  {
    // Read in topic length to calculate bytes to skip over for Stream writing
    if (!readByte(this->buffer, &len))
      return 0;
    if (!readByte(this->buffer, &len))
      return 0;
    skip = (this->buffer[*lengthLength + 1] << 8) + this->buffer[*lengthLength + 2];
    start = 2;
    if (this->buffer[0] & MQTTQOS1)
    {
      // skip message id
      skip += 2;
    }
  }
  uint32_t idx = len;

  for (uint32_t i = start; i < length; i++)
  {
    if (!readByte(&digit))
      return 0;
    if (this->stream)
    {
      if (isPublish && idx - *lengthLength - 2 > skip)
      {
        this->stream->write(digit);
      }
    }

    if (len < this->bufferSize)
    {
      this->buffer[len] = digit;
      len++;
    }
    idx++;
  }

  if (!this->stream && idx > this->bufferSize)
  {
    len = 0; // This will cause the packet to be ignored.
  }
  return len;
}

boolean OTAUpdateManager::loop()
{
  if (connected())
  {
    unsigned long t = millis();
    if ((t - lastInActivity > this->keepAlive * 1000UL) || (t - lastOutActivity > this->keepAlive * 1000UL))
    {
      if (pingOutstanding)
      {
        this->_state = MQTT_CONNECTION_TIMEOUT;
        _client->stop();
        return false;
      }
      else
      {
        this->buffer[0] = MQTTPINGREQ;
        this->buffer[1] = 0;
        _client->write(this->buffer, 2);
        lastOutActivity = t;
        lastInActivity = t;
        pingOutstanding = true;
      }
    }
    if (_client->available())
    {
      uint8_t llen;
      uint16_t len = readPacket(&llen);
      uint16_t msgId = 0;
      uint8_t *payload;
      if (len > 0)
      {
        lastInActivity = t;
        uint8_t type = this->buffer[0] & 0xF0;
        if (type == MQTTPUBLISH)
        {
          if (true) // callback
          {
            uint16_t tl = (this->buffer[llen + 1] << 8) + this->buffer[llen + 2]; /* topic length in bytes */
            memmove(this->buffer + llen + 2, this->buffer + llen + 3, tl);        /* move topic inside buffer 1 byte to front */
            this->buffer[llen + 2 + tl] = 0;                                      /* end the topic as a 'C' string with \x00 */
            char *topic = (char *)this->buffer + llen + 2;
            // msgId only present for QOS>0
            if ((this->buffer[0] & 0x06) == MQTTQOS1)
            {
              msgId = (this->buffer[llen + 3 + tl] << 8) + this->buffer[llen + 3 + tl + 1];
              payload = this->buffer + llen + 3 + tl + 2;
              callback(topic, payload, len - llen - 3 - tl - 2);

              this->buffer[0] = MQTTPUBACK;
              this->buffer[1] = 2;
              this->buffer[2] = (msgId >> 8);
              this->buffer[3] = (msgId & 0xFF);
              _client->write(this->buffer, 4);
              lastOutActivity = t;
            }
            else
            {
              payload = this->buffer + llen + 3 + tl;
              callback(topic, payload, len - llen - 3 - tl);
            }
          }
        }
        else if (type == MQTTPINGREQ)
        {
          this->buffer[0] = MQTTPINGRESP;
          this->buffer[1] = 0;
          _client->write(this->buffer, 2);
        }
        else if (type == MQTTPINGRESP)
        {
          pingOutstanding = false;
        }
      }
      else if (!connected())
      {
        // readPacket has closed the connection
        return false;
      }
    }
    return true;
  }
  return false;
}

boolean OTAUpdateManager::publish(const char *topic, const char *payload_c)
{
  unsigned int plength = payload_c ? strnlen(payload_c, this->bufferSize) : 0;
  boolean retained = false;
  const uint8_t *payload = (const uint8_t *)payload_c;

  if (connected())
  {
    if (this->bufferSize < MQTT_MAX_HEADER_SIZE + 2 + strnlen(topic, this->bufferSize) + plength)
    {
      // Too long
      return false;
    }
    // Leave room in the buffer for header and variable length field
    uint16_t length = MQTT_MAX_HEADER_SIZE;
    length = writeString(topic, this->buffer, length);

    // Add payload
    uint16_t i;
    for (i = 0; i < plength; i++)
    {
      this->buffer[length++] = payload[i];
    }

    // Write the header
    uint8_t header = MQTTPUBLISH;
    if (retained)
    {
      header |= 1;
    }
    return write(header, this->buffer, length - MQTT_MAX_HEADER_SIZE);
  }
  return false;
}

size_t OTAUpdateManager::buildHeader(uint8_t header, uint8_t *buf, uint16_t length)
{
  uint8_t lenBuf[4];
  uint8_t llen = 0;
  uint8_t digit;
  uint8_t pos = 0;
  uint16_t len = length;
  do
  {

    digit = len & 127; // digit = len %128
    len >>= 7;         // len = len / 128
    if (len > 0)
    {
      digit |= 0x80;
    }
    lenBuf[pos++] = digit;
    llen++;
  } while (len > 0);

  buf[4 - llen] = header;
  for (int i = 0; i < llen; i++)
  {
    buf[MQTT_MAX_HEADER_SIZE - llen + i] = lenBuf[i];
  }
  return llen + 1; // Full header size is variable length bit plus the 1-byte fixed header
}

boolean OTAUpdateManager::write(uint8_t header, uint8_t *buf, uint16_t length)
{
  uint16_t rc;
  uint8_t hlen = buildHeader(header, buf, length);

#ifdef MQTT_MAX_TRANSFER_SIZE
  uint8_t *writeBuf = buf + (MQTT_MAX_HEADER_SIZE - hlen);
  uint16_t bytesRemaining = length + hlen; // Match the length type
  uint8_t bytesToWrite;
  boolean result = true;
  while ((bytesRemaining > 0) && result)
  {
    bytesToWrite = (bytesRemaining > MQTT_MAX_TRANSFER_SIZE) ? MQTT_MAX_TRANSFER_SIZE : bytesRemaining;
    rc = _client->write(writeBuf, bytesToWrite);
    result = (rc == bytesToWrite);
    bytesRemaining -= rc;
    writeBuf += rc;
  }
  return result;
#else
  rc = _client->write(buf + (MQTT_MAX_HEADER_SIZE - hlen), length + hlen);
  lastOutActivity = millis();
  return (rc == hlen + length);
#endif
}

boolean OTAUpdateManager::subscribe(const char *topic)
{
  uint8_t qos = 0;
  size_t topicLength = strnlen(topic, this->bufferSize);
  if (connected())
  {
    // Leave room in the buffer for header and variable length field
    uint16_t length = MQTT_MAX_HEADER_SIZE;
    nextMsgId++;
    if (nextMsgId == 0)
    {
      nextMsgId = 1;
    }
    this->buffer[length++] = (nextMsgId >> 8);
    this->buffer[length++] = (nextMsgId & 0xFF);
    length = writeString((char *)topic, this->buffer, length);
    this->buffer[length++] = qos;
    return write(MQTTSUBSCRIBE | MQTTQOS1, this->buffer, length - MQTT_MAX_HEADER_SIZE);
  }
  return false;
}

uint16_t OTAUpdateManager::writeString(const char *string, uint8_t *buf, uint16_t pos)
{
  const char *idp = string;
  uint16_t i = 0;
  pos += 2;
  while (*idp)
  {
    buf[pos++] = *idp++;
    i++;
  }
  buf[pos - i - 2] = (i >> 8);
  buf[pos - i - 1] = (i & 0xFF);
  return pos;
}

boolean OTAUpdateManager::connected()
{
  boolean rc;
  if (_client == NULL)
  {
    rc = false;
  }
  else
  {
    rc = (int)_client->connected();
    if (!rc)
    {
      if (this->_state == MQTT_CONNECTED)
      {
        this->_state = MQTT_CONNECTION_LOST;
        _client->flush();
        _client->stop();
      }
    }
    else
    {
      return this->_state == MQTT_CONNECTED;
    }
  }
  return rc;
}

OTAUpdateManager &OTAUpdateManager::setServer(const char *domain, uint16_t port)
{
  this->domain = domain;
  this->port = port;
  return *this;
}

OTAUpdateManager &OTAUpdateManager::setClient(Client &client)
{
  this->_client = &client;
  return *this;
}

int OTAUpdateManager::state()
{
  return this->_state;
}

boolean OTAUpdateManager::setBufferSize(uint16_t size)
{
  if (size == 0)
  {
    // Cannot set it back to 0
    return false;
  }
  if (this->bufferSize == 0)
  {
    this->buffer = (uint8_t *)malloc(size);
  }
  else
  {
    uint8_t *newBuffer = (uint8_t *)realloc(this->buffer, size);
    if (newBuffer != NULL)
    {
      this->buffer = newBuffer;
    }
    else
    {
      return false;
    }
  }
  this->bufferSize = size;
  return (this->buffer != NULL);
}

OTAUpdateManager &OTAUpdateManager::setKeepAlive(uint16_t keepAlive)
{
  this->keepAlive = keepAlive;
  return *this;
}
OTAUpdateManager &OTAUpdateManager::setSocketTimeout(uint16_t timeout)
{
  this->socketTimeout = timeout;
  return *this;
}

// constructor
OTAUpdateManager::OTAUpdateManager()
{
  return;
}
OTAUpdateManager::OTAUpdateManager(const String &user, const String &token) : OTAUpdateManager(user, token, ApnOn, "Mini project", "raghulrajg")
{
  return;
}

OTAUpdateManager::OTAUpdateManager(const String &user, const String &token, const int Status, char const *ssid, char const *Password)
{
  _User = user;
  _Token = token;

  banner();
  deviceId();
  instance->connection(client);

  host = _Server + "?user=" + _User + "&token=" + _Token + "&deviceid=" + String(uniqueId);
  pinMode(2, OUTPUT);
  digitalWrite(2, HIGH);
  if (Status == ApnOn)
  {
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
  instance->setServer(mqtt_server.c_str(), mqtt_port);
  while (!instance->connected())
  {
    if (instance->connect(uniqueId))
    {
#ifdef WM_DEBUG_LEVEL
      DEBUG_WM(DEBUG_NOTIFY, F("Server is Connected"));
#endif
    }
    else
    {
#ifdef WM_DEBUG_LEVEL
      DEBUG_WM(DEBUG_ERROR, F("failed with state "), instance->state());
#endif
      delay(2000);
    }
  }

  String topic = "readytoupdate/" + _User + "/" + _Token;
  instance->subscribe(topic.c_str());

  xTaskCreateUniversal(
      OTAUpdateManager::loop,
      "loop2",
      8000,
      NULL,
      ESP_TASKD_EVENT_PRIO - 1,
      &_arduino_event_task_handle,
      ARDUINO_EVENT_RUNNING_CORE);
}

OTAUpdateManager::~OTAUpdateManager()
{
  free(this->buffer);
}

char OTAUpdateManager::uniqueId[24] = "";
String OTAUpdateManager::jsonBuffer = "";
String OTAUpdateManager::_User = "";
String OTAUpdateManager::_Token = "";
String OTAUpdateManager::host = "";
const String OTAUpdateManager::_Server = "http://firmware.serveo.net/download";
const String OTAUpdateManager::mqtt_server = "serveo.net";
const int OTAUpdateManager::mqtt_port = 2512;
int OTAUpdateManager::_StatusCode = 0;

void OTAUpdateManager::loop(void *pvParameters)
{
  while (1)
  {
    OTAUpdateManager::OTAUpdate();
  }
}

// DEBUG
// @todo fix DEBUG_WM(0,0);
template <typename Generic>
void OTAUpdateManager::DEBUG_WM(Generic text)
{
  DEBUG_WM(DEBUG_NOTIFY, text, "");
}

template <typename Generic>
void OTAUpdateManager::DEBUG_WM(wm_debuglevel_t level, Generic text)
{
  if (_debugLevel >= level)
    DEBUG_WM(level, text, "");
}

template <typename Generic, typename Genericb>
void OTAUpdateManager::DEBUG_WM(Generic text, Genericb textb)
{
  DEBUG_WM(DEBUG_NOTIFY, text, textb);
}

template <typename Generic, typename Genericb>
void OTAUpdateManager::DEBUG_WM(wm_debuglevel_t level, Generic text, Genericb textb)
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

void OTAUpdateManager::banner()
{
#ifdef WM_DEBUG_LEVEL
  DEBUG_WM(DEBUG_NOTIFY, F(DEBUG_NEWLINE
                           "    ███████     ███████████   █████████                    █████████" DEBUG_NEWLINE
                           "  ███░░░░░███  ░█░░░███░░░█  ███░░░░░███                  ███░░░░░███" DEBUG_NEWLINE
                           " ███     ░░███ ░   ░███  ░  ░███    ░███                 ███     ░░░  ████████" DEBUG_NEWLINE
                           "░███      ░███     ░███     ░███████████                ░███         ░░███░░███" DEBUG_NEWLINE
                           "░███      ░███     ░███     ░███░░░░░███                ░███    █████ ░███ ░░░" DEBUG_NEWLINE
                           "░░███     ███      ░███     ░███    ░███                ░░███  ░░███  ░███" DEBUG_NEWLINE
                           " ░░░███████░       █████    █████   █████    ██ ██ ██    ░░█████████  █████" DEBUG_NEWLINE
                           "   ░░░░░░░        ░░░░░    ░░░░░   ░░░░░    ░░ ░░ ░░      ░░░░░░░░░  ░░░░░" DEBUG_NEWLINE));
  // DEBUG_WM(DEBUG_NOTIFY, F(DEBUG_NEWLINE
  //                          " #######  ########    ###                       ######   ########" DEBUG_NEWLINE
  //                          "##     ##    ##      ## ##                     ##    ##  ##     ##" DEBUG_NEWLINE
  //                          "##     ##    ##     ##   ##                    ##        ##     ##" DEBUG_NEWLINE
  //                          "##     ##    ##    ##     ##                   ##   #### ########" DEBUG_NEWLINE
  //                          "##     ##    ##    #########                   ##    ##  ##   ##" DEBUG_NEWLINE
  //                          "##     ##    ##    ##     ##    ### ### ###    ##    ##  ##    ##" DEBUG_NEWLINE
  //                          " #######     ##    ##     ##    ### ### ###     ######   ##     ##" DEBUG_NEWLINE));
#endif
}

void OTAUpdateManager::deviceId(void)
{
  uint8_t mac[6];
  esp_read_mac(mac, ESP_MAC_WIFI_STA);

  uint64_t chipId = ESP.getEfuseMac();

  snprintf(uniqueId, sizeof(uniqueId), "%04X%08X%02X%02X%02X%02X",
           (uint16_t)(chipId >> 32),
           (uint32_t)chipId,
           mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
}
