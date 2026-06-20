#define BLYNK_TEMPLATE_ID "TMPL2grVroM2L"
#define BLYNK_TEMPLATE_NAME "LED"
#define BLYNK_AUTH_TOKEN "_1G-pvBF2hPhJFonha-P9gcWOeT09yiv"

#define BLYNK_PRINT Serial

#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>
#include "movement_states.h"

// Network credentials
char ssid[] = "Orange-mtsasb";
char pass[] = "Mt$a$b1964";

// Blynk auth token
char auth[] = "_1G-pvBF2hPhJFonha-P9gcWOeT09yiv";

const int pin = 2;

MovementStates movement_states = {0, 0, 0, 0};

extern "C"
{
  void spider();
  void reset_servos();
}

BLYNK_WRITE(V0)
{ // Button 1 - Move Forward
  (movement_states.forward = param.asInt()) ? (void)0 : reset_servos();
}

BLYNK_WRITE(V1)
{ // Button 2 - Move Backward
  (movement_states.backward = param.asInt()) ? (void)0 : reset_servos();
}

BLYNK_WRITE(V2)
{ // Button 3 - Rotate Right
  (movement_states.right = param.asInt()) ? (void)0 : reset_servos();
}

BLYNK_WRITE(V4)
{ // Button 4 - Rotate Left
  (movement_states.left = param.asInt()) ? (void)0 : reset_servos();
}

void setup()
{
  Serial.begin(115200);

  // Set GPIO 4 as output
  pinMode(pin, OUTPUT);
  digitalWrite(pin, LOW);
  
  //Blynk.begin(auth, ssid, pass);

  spider();
}

void loop()
{
  //Blynk.run();
  // Serial.print("Forward: ");
  // Serial.print(movement_states.forward);
  // Serial.print(" | Backward: ");
  // Serial.print(movement_states.backward);
  // Serial.print(" | Rotate Right: ");
  // Serial.print(movement_states.right);
  // Serial.print(" | Rotate Left: ");
  // Serial.println(movement_states.left);

  //delay(500);
}
