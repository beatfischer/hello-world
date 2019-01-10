#include <TheThingsNode.h>
#include <TheThingsNetwork.h>

// Set your DevAddr, NwkSKey, AppSKey and the frequency plan
const char *devAddr = "26011E61";
const char *nwkSKey = "4B5B2D9E182508FEB4014E55FE52F40E";
const char *appSKey = "8118BD2E90721A833C02B3F1D6B2CB23";
#define freqPlan TTN_FP_EU868

#define loraSerial Serial1
#define debugSerial Serial

TheThingsNetwork ttn(loraSerial, debugSerial, freqPlan);
TheThingsNode *node;

// Port definition
#define PORT_SETUP 1
#define PORT_INTERVAL 2
#define PORT_MOTION 3
#define PORT_BUTTON 4

// Interval between send in seconds
#define CONFIG_INTERVAL ((uint32_t) 300)

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
  node->configInterval(true, CONFIG_INTERVAL*1000);
  // node->configUSB(false);

  node->onWake(wake);
  node->onSleep(sleep);
  node->onInterval(interval);
  node->onTemperature(temperature);
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
  debugSerial.println("-- INFO: WAKE");
}

void sleep()
{
  node->setColor(TTN_BLACK);
  debugSerial.println("-- INFO: SLEEP");
}

void interval()
{
  node->setColor(TTN_GREEN);
  debugSerial.println("-- SEND: INTERVAL");
  sendData(PORT_INTERVAL);
}

void temperature()
{
  debugSerial.print("-- INFO: TEMPERATURE");
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
  sendData(PORT_MOTION);
}

void buttonPress()
{
  node->setColor(TTN_BLUE);
  debugSerial.print("-- SEND: BUTTON_PRESS");
  sendData(PORT_BUTTON);
}

void buttonRelease(unsigned long duration)
{
  node->setColor(TTN_BLACK);
  debugSerial.print("-- INFO: BUTTON_RELEASE");
  debugSerial.println(duration);
  sendData(PORT_BUTTON);
}

void sendData(uint8_t port)
{
  // Wake LoRaWAN module and show LoRaWAN and Node status
  ttn.wake();
  ttn.showStatus();
  node->showStatus();

  // Prepare payload
  byte payload[6];

  uint16_t battery = node->getBattery();
  debugSerial.println(battery);
  payload[0] = highByte(battery);
  payload[1] = lowByte(battery);

  uint16_t light = node->getLight();
  debugSerial.println(light);
  payload[2] = highByte(light);
  payload[3] = lowByte(light);

  int16_t temperature = round(node->getTemperatureAsFloat() * 100);
  debugSerial.println(temperature);
  payload[4] = highByte(temperature);
  payload[5] = lowByte(temperature);

  // Send payload
  ttn.sendBytes(payload, sizeof(payload), port);

  // Set asleep LoRaWAN module
  ttn.sleep(CONFIG_INTERVAL*1000);
  // This one is not optional, remove it and say bye bye to RN2983 sleep mode
  delay(50);
}
