#ifndef miniProject_h
#define miniProject_h

#include "stringen.h"
#include <WiFiManager.h>
#include <Arduino.h>
#include <HTTPClient.h>
#include <HTTPUpdate.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include "esp_system.h"
#include "esp_wifi.h"

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

	// private mqtt object creating
	static PubSubClient mqttclient;

	// functions
	static void loop2(void *pvParameters);
	static void OTAUpdate();
	static String httpGETRequest(miniProject *instance, const char *serverName);

public:
	// functions
	miniProject();
	miniProject(const String &user, const String &token);
	miniProject(const String &user, const String &token, const int Status, char const *ssid, char const *Password);
	static void wifimanagerConn(char const *apName, char const *apPassword, miniProject *instance);
	void deviceId(void);
	static void mqtt(int cur, int total);
	friend void callback(char *topic, byte *payload, unsigned int length);

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
