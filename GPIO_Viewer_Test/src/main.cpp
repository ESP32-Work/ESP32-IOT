#include <gpio_viewer.h> // Must me the first include in your project
GPIOViewer gpio_viewer;

const char *ssid = "KytherTek";
const char *password = "x11y22z33";

#define DEMO_PIN  18

GPIOViewer gpioViewer;
bool pinState = false;

void setup() {
  Serial.begin(115200);
  Serial.setDebugOutput(true);    // send ESP inbuilt log messages to Serial
  
  pinMode(DEMO_PIN, OUTPUT);

  gpioViewer.connectToWifi(ssid, password);
  gpioViewer.setSamplingInterval(125);
  gpioViewer.begin();
}

void loop() {
  pinState = !pinState;
  digitalWrite(DEMO_PIN, pinState);
  log_i("Current pin state: %d", pinState);
  delay(1000);
}