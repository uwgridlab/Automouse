/*  Author: James Wu
 *  For Gridlab DBS experiments
 */

#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Mouse.h>

#define BUTTON_UP 9
#define BUTTON_MODE 6
#define BUTTON_DOWN 5

Adafruit_SSD1306 display = Adafruit_SSD1306();
#if (SSD1306_LCDHEIGHT != 32)
 #error("Height incorrect, please fix Adafruit_SSD1306.h!");
#endif

// Control variables
unsigned int chmode = 1; // mode switcher variable
unsigned int pulses = 1; // max pulses when running; control variable
float iti = 1.0; // inter-trial interval in seconds; control variable

// Runtime variables
unsigned int pulseno = 0; // pulse delivery counter when running
char outstr[8]; // 8-char holder to display iti when float -> str
unsigned long debounce_last = 0; // debounce timer for buttons in millis

void setup() {
  
  delay(1000); // Note: boot sequence doesn't work right without initial delay
  
  pinMode(BUTTON_UP, INPUT_PULLUP);
  pinMode(BUTTON_MODE, INPUT_PULLUP);
  pinMode(BUTTON_DOWN, INPUT_PULLUP);

  // Init display
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);  // initialize with the I2C addr 0x3C (for the 128x32)
  display.display();
  delay(1000);
  display.clearDisplay();
  display.display();

  // Draw startup screen
  display.setTextSize(1);
  display.setTextColor(WHITE);
  redraw(chmode);
  display.display(); // actually display all of the above
}

void loop() {
  int pressed = 0;
  if (! digitalRead(BUTTON_UP)) {
    if( (millis() - debounce_last) >= 75 ) pressed = 1;
    debounce_last = millis();
  }
  else {
    if (! digitalRead(BUTTON_MODE)) {
      if( (millis() - debounce_last) >= 75 ) pressed = 2;
      debounce_last = millis();
    }
    else if (! digitalRead(BUTTON_DOWN)) {
      if( (millis() - debounce_last) >= 75 ) pressed = 3;
      debounce_last = millis();
    }
  }
  
  if (pressed == 2) {
    if (chmode == 1) chmode = 2;
    else chmode = 1;
    redraw(chmode);
  }
  else {
    if (pressed == 1) {
      change(chmode, 1); 
    }
    else if (pressed == 3) {
      change(chmode, -1);
    }
  }  
  yield();
  display.display();
}

void change(int chmode, int chdir) {
  if (chmode == 1) {
    if ( (pulses + chdir) <= 10000 && (pulses + chdir) >= 1 ) {
      pulses += chdir;
      redraw(chmode);
    }
  }
  else if (chmode == 2) {
    if ( (iti + chdir*0.1) > 0 && (iti + chdir*0.1) <= 60 ) {
      iti += chdir*0.1;
      redraw(chmode);
    }
  }
}

void redraw(int chmode) {
  display.clearDisplay();
  display.setCursor(0,0);
  display.println("DBS Mode: STOPPED\n");
//  display.fillRect(0, 2*8, 128, 2*8, BLACK);
//  display.setCursor(0, 2*8);
  display.print(pulseno);
  display.print(" of ");
  display.print(pulses);
  display.println(" pulses");
  display.print("@ ");
  dtostrf(iti, 4, 3, outstr);
  display.print(outstr);
  display.println(" sec ITI");

  display.setCursor(120, (chmode+1)*8);
  display.print('<');
}

