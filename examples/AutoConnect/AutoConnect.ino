#include <OTAUpdateManager.h>

//Avoid the GPIO pin number 2 because of predefine pin
//create your User ID and Token in https://ota.serveo.net/

//server connection config
const String& User = "YOUR_USER_ID";
const String& Token = "YOUR_TOKEN";

// put your variable here:

void setup() {
  Serial.begin(115200);
  OTAUpdateManager(User, Token); //turn on the wifi connected to miniProject and password is raghulrajg
  // put your setup code here, to run once:

} 

void loop() {
  // put your main code here, to run repeatedly:

}
