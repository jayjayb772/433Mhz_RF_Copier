#include <RCSwitch.h>
#include <EEPROM.h>
#include <U8x8lib.h>

#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

// Declaration for SSD1306 display connected using software SPI (default case):
#define OLED_MOSI   D7
#define OLED_CLK   D5
#define OLED_DC    D2
#define OLED_CS    D8
#define OLED_RESET D3
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT,
  OLED_MOSI, OLED_CLK, OLED_DC, OLED_RESET, OLED_CS);

/* Comment out above, uncomment this block to use hardware SPI
#define OLED_DC     6
#define OLED_CS     7
#define OLED_RESET  8
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT,
  &SPI, OLED_DC, OLED_RESET, OLED_CS);
*/

#define NUMFLAKES     10 // Number of snowflakes in the animation example

#define LOGO_HEIGHT   16
#define LOGO_WIDTH    16
static const unsigned char PROGMEM logo_bmp[] =
{ 0b00000000, 0b11000000,
  0b00000001, 0b11000000,
  0b00000001, 0b11000000,
  0b00000011, 0b11100000,
  0b11110011, 0b11100000,
  0b11111110, 0b11111000,
  0b01111110, 0b11111111,
  0b00110011, 0b10011111,
  0b00011111, 0b11111100,
  0b00001101, 0b01110000,
  0b00011011, 0b10100000,
  0b00111111, 0b11100000,
  0b00111111, 0b11110000,
  0b01111100, 0b11110000,
  0b01110000, 0b01110000,
  0b00000000, 0b00110000 };

#define EEPROM_SIZE 512
#define NUM_SLOTS 5
#define NUM_SELECTIONS 3
RCSwitch mySwitch = RCSwitch();

const int upButton = D0;
const int rightButton = D1;
const int downButton = D4;c dssz
const int leftButton = 3;

int upState = LOW;
int downState = LOW;
int leftState = LOW;
int rightState = LOW;

unsigned long lastDebounceTime = 0;
unsigned long debounceDelay = 50;

const int EEPROM_ADDRESSES[5] = {0, 100, 200, 300, 400};
int EEPROM_VALUES[5] = {};

const String menuOptions[] = {"Record", "Playback", "Raw Data"};
const String saveOptions[] = {"1", "2", "3", "4", "5"};

int selection = 1;
int saveSelection = 1;

int actionMode = 0;
bool inRecord = false;
bool inPlayback = false;

bool debug = true;


void setup() {
  Serial.begin(9600);
  EEPROM.begin(EEPROM_SIZE);
  pinMode(upButton, INPUT_PULLUP);
  pinMode(downButton, INPUT_PULLUP);
  pinMode(rightButton, INPUT_PULLUP);
  pinMode(leftButton, INPUT_PULLUP);


  mySwitch.enableReceive(0);  // Receiver on interrupt 0 => that is pin #2
  mySwitch.enableTransmit(10);

  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if(!display.begin(SSD1306_SWITCHCAPVCC)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }


  // Show initial display buffer contents on the screen --
  // the library initializes this with an Adafruit splash screen.
  display.display();
  delay(2000); // Pause for 2 seconds

  // Clear the buffer
  display.clearDisplay();
    display.setTextSize(1);      // Normal 1:1 pixel scale
  display.setTextColor(SSD1306_WHITE); // Draw white text
}

void debugPrint(String toPrint){
  if(debug){
    Serial.println(toPrint);
  }
}

void loadEEPROMValues() {
  for (int i = 0; i < NUM_SLOTS; i++) {
    EEPROM_VALUES[i] = EEPROM.read(EEPROM_ADDRESSES[i]);
  }
}

bool readButtonPress(int buttonPin, int *lastButtonState) {
  int reading = digitalRead(buttonPin);

  if(reading == HIGH){
    delay(50);
//    Serial.println((String)buttonPin + "high");
    return false;
  }else{
    delay(100);
    Serial.println((String)buttonPin + "low");
    return true;
  }
//  if (reading != *lastButtonState) {
//    lastDebounceTime = millis();
//  }
//
//  if ((millis() - lastDebounceTime) > debounceDelay) {
//    *lastButtonState = reading;
//    Serial.println((String)*lastButtonState);
//    Serial.println((String)reading);
//    Serial.println("Button Pressed");
//    return true;
//  }
}

void loop() {
  switch (actionMode) {
    case 0:
    mainMenuScreen();
      mainMenuLoop();
      break;

    case 1:
      inRecord = true;
      recordMode();
      break;

    case 2:
      inPlayback = true;
      playbackMode();
      break;
  }

}

void changeModeScreenUpdate() {

}

void mainMenuScreen() {
  //display options
  display.setCursor(0, 0);
  display.clearDisplay();
  for (int i = 0; i < NUM_SELECTIONS; i++) {
    String text = menuOptions[i];
    if (i + 1 == selection) {
      text += " +";
      //add check or indicator next to word
    }
    display.println(text);
    //display text
  }
  display.display();
}


void mainMenuLoop() {
  while (actionMode == 0) {
    if (mySwitch.available()) {
      Serial.println((String)mySwitch.getReceivedValue());
      Serial.println((String)mySwitch.getReceivedBitlength());
      Serial.println((String)mySwitch.getReceivedDelay());
      Serial.println((String)*mySwitch.getReceivedRawdata());
      Serial.println((String)mySwitch.getReceivedProtocol());
      mySwitch.resetAvailable();
    }


    //if up pressed
    if (readButtonPress(upButton, &upState)) {
      if (selection == 1) {
        selection = NUM_SELECTIONS;
      } else {
        selection -= 1;
      }
      mainMenuScreen();
    }

    //if down pressed
    if (readButtonPress(downButton, &downState)) {
      if (selection == 3) {
        selection = 1;
      } else {
        selection += 1;
      }
      mainMenuScreen();
    }

    //if left pressed
    if (readButtonPress(leftButton, &leftState)) {
      //nothing
    }

    //if right pressed
    if (readButtonPress(rightButton, &rightState)) {
      switch (selection) {
        case 1:
          actionMode = 1;
          break;
        case 2:
          actionMode = 2;
          break;
      }
    }
    delay(100);
  }
}



void recordToEEPROM(int slotNumber, int signalData) {
  EEPROM.write(EEPROM_ADDRESSES[slotNumber], signalData);
  EEPROM.commit();
}



void saveSlotsScreen() {
  Serial.println(saveSelection);
  display.setCursor(0, 0);
  display.clearDisplay();
  for (int i = 0; i < NUM_SLOTS; i++) {
    String text = saveOptions[i];
    if (i + 1 == saveSelection) {
      text += " +";
      //add check or indicator next to word
    }
    display.println(text);
    //display text
  }
  display.display();
}
void recordMode() {
  int curStep = 1;
  while (actionMode == 1) {
    //choose save slot
    
    saveSlotsScreen();
    while (curStep == 1) {
      delay(20);
      //if up pressed
      if (readButtonPress(upButton, &upState)) {
        if (saveSelection == 1) {
          saveSelection = NUM_SLOTS;
        } else {
          saveSelection -= 1;
        }
        saveSlotsScreen();
        Serial.println("Pressed up");
      }

      //if down pressed
      if (readButtonPress(downButton, &downState)) {
        if (saveSelection == 5) {
          saveSelection = 1;
        } else {
          saveSelection += 1;
        }
        saveSlotsScreen();
        Serial.println("Pressed Down");
      }

      //if left pressed
      if (readButtonPress(leftButton, &leftState)) {
        actionMode = 0;
        break;
      }

      //if right pressed
      if (readButtonPress(rightButton, &rightState)) {
        curStep = 2;
      }
    }

    //display "Press right button to record"
    
    while (curStep == 2) {
      delay(200);
      display.setCursor(0, 0);
    display.clearDisplay();
    display.println("Save Slot " + (String)saveSelection);
    display.display();
      delay(100);
      if (readButtonPress(rightButton, &rightState)) {
        curStep = 3;
      }
      if (readButtonPress(leftButton, &leftState)) {
        curStep = 1;
      }
    }
Serial.println(curStep + " record");
    

    String binaryData = "01101101";
    int millisTiming = 0;
    //record
    while (curStep == 3) {
      display.setCursor(0, 0);
    display.clearDisplay();
    display.println("Recording to " + (String)saveSelection);
    display.display();
      //record signals
      delay(5000);
      curStep = 4;
    }


    //save to save slot
    while(curStep == 4){
          display.setCursor(0, 0);
    display.clearDisplay();
    display.println("Saving signal to " + (String)saveSelection);
    display.display();
    recordToEEPROM(saveSelection-1, binaryData.toInt());

    delay(2000);
    curStep = 5;
    }

    while(curStep == 5){
          //parse data

      display.setCursor(0, 0);
    display.clearDisplay();
    display.println("Press Left to go home");
    display.println("Press Right to save more");
    display.display();
      if (readButtonPress(rightButton, &rightState)) {
        curStep = 1;
        Serial.println("Pressed right");
      }

      if (readButtonPress(leftButton, &leftState)) {
        Serial.println("Pressed left");
        actionMode = 0;
        break;
      }
    }
  }
}

void playbackMode() {
  int curStep = 1;
  while (inPlayback) {

    //Select Save
    while (curStep == 1) {
      saveSlotsScreen();
      //if up pressed
      if (readButtonPress(upButton, &upState)) {
        if (saveSelection == 1) {
          saveSelection = NUM_SLOTS;
        } else {
          saveSelection -= 1;
        }
        saveSlotsScreen();
      }

      //if down pressed
      if (readButtonPress(downButton, &downState)) {
        if (saveSelection == 5) {
          saveSelection = 1;
        } else {
          saveSelection += 1;
        }
        saveSlotsScreen();
      }

      //if left pressed
      if (readButtonPress(leftButton, &leftState)) {
        actionMode = 0;
        break;
      }

      //if right pressed
      if (readButtonPress(rightButton, &rightState)) {
        curStep = 2;
      }
    }
    
    // Playback Save
    while (curStep == 2) {
      //if left pressed
      if (readButtonPress(leftButton, &leftState)) {
        curStep == 1;
      }

      //if right pressed
      if (readButtonPress(rightButton, &rightState)) {
        //playback saved signal
      }
    }
  }
}
