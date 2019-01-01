#include <TheThingsNetwork.h>

// Set your DevAddr, NwkSKey, AppSKey and the frequency plan
const char *devAddr = "2601167F";
const char *nwkSKey = "6877E75BB929EB04AFD5F6EDA1E8D3F1";
const char *appSKey = "37536CEF69E721254D14D617131C0B39";

#define loraSerial Serial1
#define debugSerial Serial

// Replace REPLACE_ME with TTN_FP_EU868 or TTN_FP_US915
#define freqPlan TTN_FP_EU868

TheThingsNetwork ttn(loraSerial, debugSerial, freqPlan);

void setup()
{
  loraSerial.begin(57600);
  debugSerial.begin(9600);

  // Wait a maximum of 10s for Serial Monitor
  while (!debugSerial && millis() < 10000)
    ;

  debugSerial.println("-- PERSONALIZE");
  ttn.personalize(devAddr, nwkSKey, appSKey);

  debugSerial.println("-- STATUS");
  ttn.showStatus();
}

void loop()
{
  debugSerial.println("-- LOOP");

  // Prepare payload of 2 byte to indicate LED status
  byte payload[2];
  payload[0] = (digitalRead(LED_BUILTIN) == HIGH) ? 1 : 0;
  payload[1] = 1;

  // Send it off
  ttn.sendBytes(payload, sizeof(payload)), 2;

  debugSerial.println("-- STATUS");
  ttn.showStatus();

  delay(10000);
}
