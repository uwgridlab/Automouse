/*  Author: James Wu
 *  For Gridlab DBS experiments
 */

#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Mouse.h>

// Adjustment controls
#define BUTTON_UP 9
#define BUTTON_MODE 6
#define BUTTON_DOWN 5
// Run-time controls
#define BUTTON_RUN 10
#define BUTTON_STOP 11
#define SWITCH_ARM 12

#define DEBOUNCE_TIME 80
#define HOLD_TIME 500
#define RESET_HOLD_TIME 1000

#define PULSE_ADJ 1
#define ITI_ADJ 0.05

#define ACT_ADJ_UP 1
#define ACT_CH_MODE 2
#define ACT_ADJ_DOWN 3
#define ACT_RUN 4
#define ACT_STOP 5
#define ACT_ARM 6
#define ACT_RESET 9

Adafruit_SSD1306 display = Adafruit_SSD1306();
#if (SSD1306_LCDHEIGHT != 32)
 #error("Height incorrect, please fix Adafruit_SSD1306.h!");
#endif

// Control variables
int chmode = 1; // mode switcher variable
int pulses = 1; // max pulses when running; control variable
float iti = 1.0; // inter-trial interval in seconds; control variable

// Runtime variables
bool isarmed = false;
bool isrunning = false;
int pulseno = 0; // pulse delivery counter when running
char outstr[8]; // 8-char holder to display iti when float -> str
unsigned long debounce_last = 0; // debounce timer for buttons in millis
unsigned long held_last = 0; // button holding timer
int last_pressed = 0;

void setup() {
  delay(1000); // Note: boot sequence doesn't work right without boot delay
  // probably because of screen i2c communications
  
  pinMode(BUTTON_UP, INPUT_PULLUP);
  pinMode(BUTTON_MODE, INPUT_PULLUP);
  pinMode(BUTTON_DOWN, INPUT_PULLUP);
  pinMode(BUTTON_RUN, INPUT_PULLUP);
  pinMode(BUTTON_STOP, INPUT_PULLUP);
  pinMode(SWITCH_ARM, INPUT_PULLUP);

  // Init display
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);  // initialize with the I2C addr 0x3C (for the 128x32)
  display.display();
  delay(1000);
  display.clearDisplay();
  display.display();

  // Draw startup screen
  display.setTextSize(1);
  display.setTextColor(WHITE);
  redraw();
  display.display(); // actually display all of the above
}

void loop() {
  int pressed = 0;
  pressed = poll_buttons();

  if(pressed){
    switch(pressed) {
      case ACT_ADJ_UP:
        changeval(1);
        break;
      case ACT_ADJ_DOWN:
        changeval(-1);
        break;
      case ACT_CH_MODE:
        if (chmode == 1) chmode = 2;
        else chmode = 1;
        redraw();
        break;
      case ACT_RUN:
        if (isarmed)
          isrunning = true;
        break;
      case ACT_STOP:
        isrunning = false;
        break;
      case ACT_RESET:
        break;
    }
    yield();
    display.display();
  }
}

void changeval(int chdir) {
  if (chmode == 1) {
    if ( (pulses + chdir) <= 9999 && (pulses + chdir) >= 1 ) {
      pulses += chdir*PULSE_ADJ;
      redraw();
    }
  }
  else if (chmode == 2) {
    if ( (iti + chdir*ITI_ADJ) > 0 && (iti + chdir*ITI_ADJ) <= 99 ) {
      iti += (float)chdir*ITI_ADJ;
      redraw();
    }
  }
}

int poll_buttons() {
  int pressed = 0;
  if (! digitalRead(BUTTON_UP)) { // ADJUSTMENT UP, allow hold
    if( (millis() - debounce_last) >= DEBOUNCE_TIME ) {
      pressed = ACT_ADJ_UP;
      held_last = millis();
    }
    else if ((last_pressed == ACT_ADJ_UP) && (millis() - held_last >= HOLD_TIME)) pressed = ACT_ADJ_UP;
    debounce_last = millis();
  }
  if (! digitalRead(BUTTON_MODE)) { // MODE SWITCH, debounce only
    if( (millis() - debounce_last) >= DEBOUNCE_TIME ) pressed = ACT_CH_MODE;
    debounce_last = millis();
  }
  if (! digitalRead(BUTTON_DOWN)) { // ADJUSTMENT DOWN, allow hold
    if( (millis() - debounce_last) >= DEBOUNCE_TIME ) {
      pressed = ACT_ADJ_DOWN;
      held_last = millis();
    }
    else if ((last_pressed == ACT_ADJ_DOWN) && (millis() - held_last >= HOLD_TIME)) pressed = ACT_ADJ_DOWN;
    debounce_last = millis();
  }
  if (! digitalRead(BUTTON_RUN)) {
    if( (millis() - debounce_last) >= DEBOUNCE_TIME ) pressed = ACT_RUN;
    debounce_last = millis();
  }
  if (! digitalRead(BUTTON_STOP)) {
    if( (millis() - debounce_last) >= DEBOUNCE_TIME ) {
      pressed = ACT_STOP;
      held_last = millis();
    }
    else if ((last_pressed == ACT_STOP) && (millis() - held_last >= RESET_HOLD_TIME)) pressed = ACT_RESET;
    debounce_last = millis();
  }
  
  if(pressed) last_pressed = pressed;
  return pressed;
}

void redraw() {
  display.clearDisplay();
  display.setCursor(0,0);
  display.print("DBS Mode:");
  // Armed indicator
  if(isarmed) {
    display.setTextColor(INVERSE);
    display.println(" ARMED ");
  }
  else display.println(" SAFE ");
  // Running indicator
  if(isrunning) {
    display.setTextColor(INVERSE);
    display.println(" Running ");
  }
  else display.println("Stopped");
  // Pulse counter indicator
  display.print(pulseno);
  display.print(" of ");
  display.print(pulses);
  display.println(" pulses");
  // ITI indicator
  display.print("@ ");
  dtostrf(iti, 4, 3, outstr);
  display.print(outstr);
  display.println(" sec ITI");
  // Adjustment indicator
  display.setCursor(120, (chmode+1)*8);
  display.print('<');
}

