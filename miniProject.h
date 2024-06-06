#ifndef miniProject_h
#define miniProject_h

#include <WiFiManager.h>
#include <Arduino.h>
#include <HTTPClient.h>
#include <HTTPUpdate.h>
#include <ArduinoJson.h>
#include "esp_system.h"
#include "esp_wifi.h"

#define DEBUG_NEWLINE "\r\n"

#define MQTT_HEADER_VERSION_LENGTH 7
#define MQTT_MAX_PACKET_SIZE 256
#define MQTT_KEEPALIVE 15
#define MQTT_SOCKET_TIMEOUT 15

#define MQTT_CONNECTION_TIMEOUT -4
#define MQTT_CONNECTION_LOST -3
#define MQTT_CONNECT_FAILED -2
#define MQTT_DISCONNECTED -1
#define MQTT_CONNECTED 0

#define MQTTCONNECT 1 << 4		// Client request to connect to Server
#define MQTTPUBLISH 3 << 4		// Publish message
#define MQTTPUBACK 4 << 4
#define MQTTSUBSCRIBE 8 << 4
#define MQTTPINGREQ 12 << 4		// PING Request
#define MQTTPINGRESP 13 << 4
#define MQTTQOS1 (1 << 1)

#define MQTT_MAX_HEADER_SIZE 5

#define No_Update 0
#define Update 1
#define user_token_invalid -1

#define WiFiOn 1
#define ApnOn 2

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
	static String _User;
	static String _Token;
	static const String _Server;
	static String host;
	static int _StatusCode;

	Client *_client;
	uint8_t *buffer;
	uint8_t *custom_buffer;
	uint16_t bufferSize;
	uint16_t keepAlive;
	uint16_t socketTimeout;
	uint16_t nextMsgId;
	unsigned long lastOutActivity;
	unsigned long lastInActivity;
	bool pingOutstanding;
	uint32_t readPacket(uint8_t *);
	boolean readByte(uint8_t *result);
	boolean readByte(uint8_t *result, uint16_t *index);
	boolean write(uint8_t header, uint8_t *buf, uint16_t length);
	uint16_t writeString(const char *string, uint8_t *buf, uint16_t pos);
	size_t buildHeader(uint8_t header, uint8_t *buf, uint16_t length);
	const char *domain;
	uint16_t port;
	Stream *stream;
	int _state;

	// functions
	static void loop(void *pvParameters);
	static void OTAUpdate();

public:
	// functions
	void banner();
	miniProject();
	miniProject(const String &user, const String &token);
	miniProject(const String &user, const String &token, const int Status, char const *ssid, char const *Password);

	~miniProject();

	static void wifimanagerConn(char const *apName, char const *apPassword, miniProject *instance);
	void deviceId(void);
	void Alive(int cur, int total);
	friend void callback(char *topic, byte *payload, unsigned int length);

	void connection(Client &client);
	miniProject &setServer(const char *domain, uint16_t port);
	miniProject &setClient(Client &client);
	miniProject &setKeepAlive(uint16_t keepAlive);
	miniProject &setSocketTimeout(uint16_t timeout);

	boolean setBufferSize(uint16_t size);
	boolean connect(const char *id);
	boolean publish(const char *topic, const char *payload_c);
	boolean subscribe(const char *topic);

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
	String _debugPrefix = FPSTR("Gr²: ");

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
