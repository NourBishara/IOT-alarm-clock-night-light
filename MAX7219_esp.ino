#include <MD_Parola.h>
#include <MD_MAX72xx.h>
#include <SPI.h>

// Uncomment according to your hardware type
#define HARDWARE_TYPE MD_MAX72XX::FC16_HW
//#define HARDWARE_TYPE MD_MAX72XX::GENERIC_HW

// Defining size, and output pins
#define MAX_DEVICES 4
#define CS_PIN 15
#define SCK_PIN 14
#define MOSI_PIN 13

MD_Parola Display = MD_Parola(HARDWARE_TYPE,MOSI_PIN,SCK_PIN, CS_PIN, MAX_DEVICES);

void setup() {
 
  Display.begin();
  Display.setIntensity(0);
  Display.displayClear();
}

void loop() {
  Display.setTextAlignment(PA_LEFT);
  Display.print("ESP");
  delay(2000);
  
  Display.setTextAlignment(PA_CENTER);
  Display.print("ESP");
  delay(2000);

  Display.setTextAlignment(PA_RIGHT);
  Display.print("ESP");
  delay(2000);

  Display.setTextAlignment(PA_CENTER);
  Display.setInvert(true);
  Display.print("ESP");
  delay(2000);

  Display.setInvert(false);
  delay(2000);
}
