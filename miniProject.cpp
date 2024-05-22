#include "miniProject.h"

WiFiClient client;
WiFiClient httpclient;
HTTPClient http;
PubSubClient miniProject::mqttclient(client);

void update_started() {
  Serial.println("CALLBACK:  HTTP update process started");
}

void update_finished() {
  Serial.println("CALLBACK:  HTTP update process finished");
  ESP.restart();
}

void update_progress(int Cur, int Total) {
  digitalWrite(2, HIGH);
  Serial.printf("CALLBACK:  HTTP update process at %d of %d bytes...\n", Cur, Total);
  miniProject::mqtt(Cur, Total);
}

void update_error(int err) {
  Serial.printf("CALLBACK:  HTTP update fatal error code %d\n", err);
}

void callback(char *topic, byte *payload, unsigned int length) {
    char msg[length + 1];
    memcpy(msg, payload, length);
    msg[length] = '\0';
    
    StaticJsonDocument<200> doc;
    DeserializationError error = deserializeJson(doc, msg);
    if(error){
    	Serial.print("parsing failed: ");
    	Serial.println(error.c_str());
	}
	miniProject::_StatusCode = doc["status"];
//	Serial.print("inside the callback: ");
//	Serial.println(miniProject::_StatusCode);
}

void miniProject::mqtt(int cur=0, int total=0){
  StaticJsonDocument<256> doc;
  doc["deviceID"] = String(uniqueId);
  doc["tot"] = total;
  doc["cur"] = cur;
  doc["user"] = _User;
  doc["token"] = _Token;
  char out[200];
  int b =serializeJson(doc, out);
  String topic = "state/"+_User;
  if(mqttclient.publish(topic.c_str(), out)){
    //Serial.print("message is send! ");
    //Serial.println(out);
    digitalWrite(2, LOW);
  }else{
  	ESP.restart();
  }
  
//  String UpdateTopic = "readytoupdate/"+_User+"/"+_Token;
//  mqttclient.subscribe(UpdateTopic.c_str());
  
  mqttclient.loop();
}

void miniProject::wifimanagerConn(char const *apName, char const *apPassword){
 WiFiManager wm;
 Serial.begin(115200);
 bool res;
 res = wm.autoConnect(apName,apPassword);
 if(!res) {
        Serial.println("Failed to connect");
        // ESP.restart();
    } 
    else {
        //if you get here you have connected to the WiFi    
        Serial.	println("connected...yeey :)");
        digitalWrite(2, LOW);
    }
}

void miniProject::OTAUpdate(){
  WiFiClient client1;
//  PubSubClient client(client);
  Serial.begin(115200);
  miniProject::mqtt();
  //host = _Server+"?user="+_User+"&token="+_Token+"&deviceid="+String(uniqueId);
  //Serial.println(String(host));
//  Serial.print(miniProject::_StatusCode);
//  Serial.print(" ");
//  Serial.print(Update);
//  Serial.print(" ");
//  Serial.println(miniProject::_StatusCode == Update);
  if(_StatusCode == Update){
	httpUpdate.onStart(update_started);
    httpUpdate.onEnd(update_finished);
    httpUpdate.onProgress(update_progress);
    httpUpdate.onError(update_error);
    
    t_httpUpdate_return ret = httpUpdate.update(client1, host);
    
    switch (ret) {
      case HTTP_UPDATE_FAILED:
        Serial.printf("HTTP_UPDATE_FAILED Error (%d): %s\n", httpUpdate.getLastError(), httpUpdate.getLastErrorString().c_str());
        break;

      case HTTP_UPDATE_NO_UPDATES:
        Serial.println("HTTP_UPDATE_NO_UPDATES");
        break;

      case HTTP_UPDATE_OK:
        Serial.println("HTTP_UPDATE_OK");
        break;
    }
    
  }else if(_StatusCode == user_token_invalid){
  	Serial.println("invalid User or Token");
  }

  if(WiFi.status() != WL_CONNECTED){
  	delay(100);
  	Serial.println("ESP32 restart");
  	ESP.restart();
  }
}

//constructor
miniProject::miniProject(const String& user, const String& token){
	_User = user;
    _Token = token;
	
	char const *apName = "Mini project";
	char const *apPassword = "raghulrajg";
	deviceId();
	host = _Server+"?user="+_User+"&token="+_Token+"&deviceid="+String(uniqueId);
	pinMode(2, OUTPUT);
	digitalWrite(2, HIGH);
	wifimanagerConn(apName, apPassword);
	mqttclient.setServer(mqtt_server.c_str(), mqtt_port);
	mqttclient.setCallback(callback);
	while (!mqttclient.connected()) {
	    if (mqttclient.connect(uniqueId, mqtt_username.c_str(), mqtt_password.c_str())) {
	       	Serial.println("Server is Connected");
		}else {	           	
		    Serial.print("failed with state ");
	        Serial.print(mqttclient.state());
	       	delay(2000);
	    }
	}
	String topic = "readytoupdate/"+_User+"/"+_Token;
	mqttclient.subscribe(topic.c_str());
	xTaskCreatePinnedToCore(
		loop2,
		"loop2",
		8000,
		NULL,
		0,
		NULL,
		0
	);
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
	
String miniProject::httpGETRequest(const char* serverName) {
  http.begin(httpclient, serverName);
  int httpResponseCode = http.GET();
  String payload = "{}"; 
  if (httpResponseCode>0) {
//    Serial.print("HTTP Response code: ");
//    Serial.println(httpResponseCode);
    payload = http.getString();
  }
  else {
    Serial.print("Error code: ");
    Serial.println(httpResponseCode);
  }
  http.end();

  return payload;
}


void miniProject::loop2(void* pvParameters){
	while(1){
		miniProject::OTAUpdate();
	}
}

void miniProject::deviceId(void){
	uint8_t mac[6];
	esp_read_mac(mac, ESP_MAC_WIFI_STA);
  
	uint64_t chipId = ESP.getEfuseMac();
	
	snprintf(uniqueId, sizeof(uniqueId), "%04X%08X%02X%02X%02X%02X", 
				(uint16_t)(chipId >> 32),
                (uint32_t)chipId,
                mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);  
}
