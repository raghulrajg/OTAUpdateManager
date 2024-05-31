#include <miniProject.h>

//Avoid the GPIO pin number 2 because of predefine pin
//create your User ID and Token in https://ota.serveo.net/
 
const String& User = "YOUR_USER_ID";
const String& Token = "YOUR_TOKEN";

// put your variable here:
char const *apName = "Mini project";
char const *apPassword = "raghulrajg";

void setup() {
  Serial.begin(115200);
  miniProject(User, Token, ApnOn, apName, apPassword);
  // put your setup code here, to run once:

} 

void loop() {
  // put your main code here, to run repeatedly:

}