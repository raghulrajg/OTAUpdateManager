#include <miniProject.h>

//Avoid the GPIO pin number 2 because of predefine pin
//create your User ID and Token in https://ota.serveo.net/

const String& User = "YOUR_USER_ID";
const String& Token = "YOUR_TOKEN";

//Put Your Wifi Name and Password
char const *SSID = "Mini project";
char const *Password = "raghulrajg";

// put your variable here:

void setup() {
  Serial.begin(115200);
  miniProject(User, Token, WiFiOn, SSID, Password);
  // put your setup code here, to run once:

} 

void loop() {
  // put your main code here, to run repeatedly:

}
