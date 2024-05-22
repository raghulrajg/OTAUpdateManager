#ifndef miniProject_h
#define miniProject_h

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


void update_started();
void update_finished();
void update_progress(int cur, int total);
void update_error(int err);
void callback(char *topic, byte *payload, unsigned int length);

class miniProject{
	private:
		//variable
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
		//static int _StatusCode;
		static int _StatusCode;
		
		//private mqtt object creating
		static PubSubClient mqttclient;
		
		//functions
		static void loop2 (void* pvParameters);
		static void OTAUpdate(void);
		static String httpGETRequest(const char* serverName);
		
	public:
		//functions 
		miniProject(const String& user, const String& token);
		static void wifimanagerConn(char const *apName, char const *apPassword);
		void deviceId(void);
		static void mqtt(int cur, int total);
		friend void callback(char *topic, byte *payload, unsigned int length);
};
#endif
