#include <TheThingsNode.h>

// Set your DevAddr, NwkSKey, AppSKey and the frequency plan
const char *devAddr = "26011CC4";
const char *nwkSKey = "120059049B852670EAC5E7AD2E5E035E";
const char *appSKey = "880877AF3DBB9FF1379FA792FADB2718";
#define freqPlan TTN_FP_EU868

#define loraSerial Serial1
#define debugSerial Serial

TheThingsNetwork ttn(loraSerial, debugSerial, freqPlan);
TheThingsNode *node;

#define PORT_SETUP 1
#define PORT_INTERVAL 2
#define PORT_MOTION 3
#define PORT_BUTTON 4

/*
Decoder payload function
------------------------

function Decoder(bytes, port) {
  var decoded = {};
  var events = {
    1: 'setup',
    2: 'interval',
    3: 'motion',
    4: 'button'
  };
  decoded.event = events[port];
  decoded.battery = (bytes[0] << 8) + bytes[1];
  decoded.light = (bytes[2] << 8) + bytes[3];
  if (bytes[4] & 0x80)
    decoded.temperature = ((0xffff << 16) + (bytes[4] << 8) + bytes[5]) / 100;
  else
    decoded.temperature = ((bytes[4] << 8) + bytes[5]) / 100;
  return decoded;
}
*/

void setup()
{
  loraSerial.begin(57600);
  debugSerial.begin(9600);

  // Wait a maximum of 10s for Serial Monitor
  while (!debugSerial && millis() < 10000)
    ;

  // Config Node
  // Initial Node setup
  node = TheThingsNode::setup();
  node->configInterval(true, 600000);
  node->configLight(true);
  node->configTemperature(true);
  node->configMotion(false);
  node->configButton(false);
  node->configUSB(false);

  node->onWake(wake);
  node->onSleep(sleep);
  node->onInterval(interval);
  node->onMotionStart(motionStart);
  node->onMotionStop(motionStop);
  node->onButtonPress(buttonPress);
  node->onButtonRelease(buttonRelease);

  // Test sensors and set LED to GREEN if it works
  node->showStatus();
  node->setColor(TTN_GREEN);

  debugSerial.println("-- TTN: STATUS");
  ttn.showStatus();

  debugSerial.println("-- TTN: PERSONALIZE");
  ttn.personalize(devAddr, nwkSKey, appSKey);

  debugSerial.println("-- SEND: SETUP");
  sendData(PORT_SETUP);
}

void loop()
{
  node->loop();
}

void wake()
{
  node->setColor(TTN_GREEN);

  // Wake LoRaWAN module and show LoRaWAN and Node status
  ttn.wake();
  ttn.showStatus();
  node->showStatus();
}

void sleep()
{
  node->setColor(TTN_BLACK);

  // Set asleep LoRaWAN module
  ttn.sleep(6000000);
  // This one is not optional, remove it and say bye bye to RN2983 sleep mode
  delay(50);
}

void interval()
{
  node->setColor(TTN_GREEN);

  debugSerial.println("-- SEND: INTERVAL");
  sendData(PORT_INTERVAL);
}

void motionStart()
{
  node->setColor(TTN_BLUE);

  debugSerial.print("-- SEND: MOTION_START");
  sendData(PORT_MOTION);
}

void motionStop()
{
  node->setColor(TTN_BLACK);

  debugSerial.print("-- INFO: MOTION_STOP");
}

void buttonPress(unsigned long duration)
{
  node->setColor(TTN_BLUE);

  debugSerial.print("-- SEND: BUTTON_PRESS");
  debugSerial.println(duration);
  sendData(PORT_BUTTON);
}

void buttonRelease(unsigned long duration)
{
  node->setColor(TTN_BLACK);

  debugSerial.print("-- INFO: BUTTON_RELEASE");
  debugSerial.println(duration);
}

void sendData(uint8_t port)
{
  byte *bytes;
  byte payload[6];

  uint16_t battery = node->getBattery();
  debugSerial.println(battery);
  bytes = (byte *)&battery;
  payload[0] = bytes[1];
  payload[1] = bytes[0];

  uint16_t light = node->getLight();
  debugSerial.println(light);
  bytes = (byte *)&light;
  payload[2] = bytes[1];
  payload[3] = bytes[0];

  int16_t temperature = round(node->getTemperatureAsFloat() * 100);
  debugSerial.println(temperature);
  bytes = (byte *)&temperature;
  payload[4] = bytes[1];
  payload[5] = bytes[0];

  ttn.sendBytes(payload, sizeof(payload), port);
}
