#ifndef miniProject_h
#define miniProject_h

#include "stringen.h"
#include <WiFiManager.h>
#include <Arduino.h>
#include <HTTPClient.h>
#include <HTTPUpdate.h>
// #include <PubSubLib.h>
#include <ArduinoJson.h>
#include "esp_system.h"
#include "esp_wifi.h"

#define MQTT_VERSION_3_1 3
#define MQTT_VERSION_3_1_1 4

// MQTT_VERSION : Pick the version
// #define MQTT_VERSION MQTT_VERSION_3_1
#ifndef MQTT_VERSION
#define MQTT_VERSION MQTT_VERSION_3_1_1
#endif

// MQTT_MAX_PACKET_SIZE : Maximum packet size. Override with setBufferSize().
#ifndef MQTT_MAX_PACKET_SIZE
#define MQTT_MAX_PACKET_SIZE 256
#endif

// MQTT_KEEPALIVE : keepAlive interval in Seconds. Override with setKeepAlive()
#ifndef MQTT_KEEPALIVE
#define MQTT_KEEPALIVE 15
#endif

// MQTT_SOCKET_TIMEOUT: socket timeout interval in Seconds. Override with setSocketTimeout()
#ifndef MQTT_SOCKET_TIMEOUT
#define MQTT_SOCKET_TIMEOUT 15
#endif

#define MQTT_CONNECTION_TIMEOUT -4
#define MQTT_CONNECTION_LOST -3
#define MQTT_CONNECT_FAILED -2
#define MQTT_DISCONNECTED -1
#define MQTT_CONNECTED 0
#define MQTT_CONNECT_BAD_PROTOCOL 1
#define MQTT_CONNECT_BAD_CLIENT_ID 2
#define MQTT_CONNECT_UNAVAILABLE 3
#define MQTT_CONNECT_BAD_CREDENTIALS 4
#define MQTT_CONNECT_UNAUTHORIZED 5

#define MQTTCONNECT 1 << 4		// Client request to connect to Server
#define MQTTCONNACK 2 << 4		// Connect Acknowledgment
#define MQTTPUBLISH 3 << 4		// Publish message
#define MQTTPUBACK 4 << 4		// Publish Acknowledgment
#define MQTTPUBREC 5 << 4		// Publish Received (assured delivery part 1)
#define MQTTPUBREL 6 << 4		// Publish Release (assured delivery part 2)
#define MQTTPUBCOMP 7 << 4		// Publish Complete (assured delivery part 3)
#define MQTTSUBSCRIBE 8 << 4	// Client Subscribe request
#define MQTTSUBACK 9 << 4		// Subscribe Acknowledgment
#define MQTTUNSUBSCRIBE 10 << 4 // Client Unsubscribe request
#define MQTTUNSUBACK 11 << 4	// Unsubscribe Acknowledgment
#define MQTTPINGREQ 12 << 4		// PING Request
#define MQTTPINGRESP 13 << 4	// PING Response
#define MQTTDISCONNECT 14 << 4	// Client is Disconnecting
#define MQTTReserved 15 << 4	// Reserved

#define MQTTQOS0 (0 << 1)
#define MQTTQOS1 (1 << 1)
#define MQTTQOS2 (2 << 1)

// Maximum size of fixed header and variable length size header
#define MQTT_MAX_HEADER_SIZE 5

// #if defined(ESP8266) || defined(ESP32)
// #include <functional>
// #define MQTT_CALLBACK_SIGNATURE std::function<void(char *, uint8_t *, unsigned int)> callback
// #else
// #define MQTT_CALLBACK_SIGNATURE void (*callback)(char *, uint8_t *, unsigned int)
// #endif

#define CHECK_STRING_LENGTH(l, s)                                \
	if (l + 2 + strnlen(s, this->bufferSize) > this->bufferSize) \
	{                                                            \
		_client->stop();                                         \
		return false;                                            \
	}

#define No_Update 0
#define Update 1
#define user_token_invalid -1

#define WiFiOn 1
#define ApnOn 2

#define WM_G(string_literal) (String(FPSTR(string_literal)).c_str())

void update_started();
void update_finished();
void update_progress(int cur, int total);
void update_error(int err);
void callback(char *topic, byte *payload, unsigned int length);

class miniProject
{
private:
	// variable
	static char uniqueId[24];
	static String jsonBuffer;
	static const String mqtt_server;
	static const int mqtt_port;
	static const String mqtt_username;
	static const String mqtt_password;
	static String _User;
	static String _Token;
	static const String _Server;
	static String host;
	// static int _StatusCode;
	static int _StatusCode;

	// mqtt variable
	Client *_client;
	uint8_t *buffer;
	uint16_t bufferSize;
	uint16_t keepAlive;
	uint16_t socketTimeout;
	uint16_t nextMsgId;
	unsigned long lastOutActivity;
	unsigned long lastInActivity;
	bool pingOutstanding;
	//MQTT_CALLBACK_SIGNATURE;
	uint32_t readPacket(uint8_t *);
	boolean readByte(uint8_t *result);
	boolean readByte(uint8_t *result, uint16_t *index);
	boolean write(uint8_t header, uint8_t *buf, uint16_t length);
	uint16_t writeString(const char *string, uint8_t *buf, uint16_t pos);
	size_t buildHeader(uint8_t header, uint8_t *buf, uint16_t length);
	IPAddress ip;
	const char *domain;
	uint16_t port;
	Stream *stream;
	int _state;

	// private mqtt object creating
	//static PubSubLib mqttclient;

	// functions
	static void loop2(void *pvParameters);
	static void OTAUpdate();
	static String httpGETRequest(miniProject *instance, const char *serverName);

public:
	// functions
	miniProject();
	// miniProject(Client& client);
	miniProject(const String &user, const String &token);
	miniProject(const String &user, const String &token, const int Status, char const *ssid, char const *Password);

	~miniProject();

	static void wifimanagerConn(char const *apName, char const *apPassword, miniProject *instance);
	void deviceId(void);
	void mqtt(int cur, int total);
	friend void callback(char *topic, byte *payload, unsigned int length);

	// mqtt
	void connection(Client &client);
	miniProject &setServer(const char *domain, uint16_t port);
	//miniProject &setCallback(MQTT_CALLBACK_SIGNATURE);
	miniProject &setClient(Client &client);
	miniProject &setKeepAlive(uint16_t keepAlive);
	miniProject &setSocketTimeout(uint16_t timeout);

	boolean setBufferSize(uint16_t size);
	boolean connect(const char *id, const char *user, const char *pass);
	boolean connect(const char *id, const char *user, const char *pass, const char *willTopic, uint8_t willQos, boolean willRetain, const char *willMessage, boolean cleanSession);
	boolean publish(const char *topic, const char *payload);
	boolean publish(const char *topic, const uint8_t *payload, unsigned int plength, boolean retained);
	virtual size_t write(uint8_t);
	virtual size_t write(const uint8_t *buffer, size_t size);
	boolean subscribe(const char *topic);
	boolean subscribe(const char *topic, uint8_t qos);

	boolean loop();
	boolean connected();
	int state();

	// debugging
	typedef enum
	{
		DEBUG_SILENT = 0,  // debug OFF but still compiled for runtime
		DEBUG_ERROR = 1,   // error only
		DEBUG_NOTIFY = 2,  // default stable,INFO
		DEBUG_VERBOSE = 3, // move verbose info
		DEBUG_DEV = 4,	   // development useful debugging info
		DEBUG_MAX = 5	   // MAX extra dev auditing, var dumps etc (MAX+1 will print timing,mem and frag info)
	} wm_debuglevel_t;

	boolean _debug = true;
	String _debugPrefix = FPSTR(S_debugPrefix);

	wm_debuglevel_t debugLvlShow = DEBUG_VERBOSE; // at which level start showing [n] level tags

// build debuglevel support
// @todo use DEBUG_ESP_x?

// Set default debug level
#ifndef WM_DEBUG_LEVEL
#define WM_DEBUG_LEVEL DEBUG_NOTIFY
#endif

// override debug level OFF
#ifdef WM_NODEBUG
#undef WM_DEBUG_LEVEL
#endif

#ifdef WM_DEBUG_LEVEL
	uint8_t _debugLevel = (uint8_t)WM_DEBUG_LEVEL;
#else
	uint8_t _debugLevel = 0; // default debug level
#endif

// @todo use DEBUG_ESP_PORT ?
#ifdef WM_DEBUG_PORT
	Print &_debugPort = WM_DEBUG_PORT;
#else
	Print &_debugPort = Serial; // debug output stream ref
#endif

	template <typename Generic>
	void DEBUG_WM(Generic text);

	template <typename Generic>
	void DEBUG_WM(wm_debuglevel_t level, Generic text);
	template <typename Generic, typename Genericb>
	void DEBUG_WM(Generic text, Genericb textb);
	template <typename Generic, typename Genericb>
	void DEBUG_WM(wm_debuglevel_t level, Generic text, Genericb textb);
};
#endif
