#include <miniProject.h>

//Avoid the GPIO pin number 2 because of predefine pin
//Do not use Serial baud Rate 250000
//create your User ID and Token in https://ota.serveo.net/

const String& User = "YOUR_USER_ID";
const String& Token = "YOUR_TOKEN";

// put your variable here:

void setup() {
  miniProject(User, Token);
  // put your setup code here, to run once:

} 

void loop() {
  // put your main code here, to run repeatedly:

}
