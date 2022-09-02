#include <RCSwitch.h>
#include <EEPROM.h>
#include <U8x8lib.h>

#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);


#define NUMFLAKES     10 // Number of snowflakes in the animation example

#define LOGO_HEIGHT   48
#define LOGO_WIDTH    48
static const unsigned char PROGMEM logo_bmp[] =
{ 
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3F, 0xFC, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF,
0x00, 0x00, 0x00, 0x07, 0xFF, 0xFF, 0xE0, 0x00, 0x00, 0x1F, 0xFC, 0x3F, 0xF8, 0x00, 0x00, 0x3F,
0xE0, 0x07, 0xFC, 0x00, 0x00, 0xFC, 0x00, 0x00, 0x3F, 0x00, 0x01, 0xF8, 0x0F, 0xF0, 0x1F, 0x80,
0x03, 0xF0, 0x3F, 0xFC, 0x0F, 0xC0, 0x07, 0x80, 0xFF, 0xFF, 0x01, 0xE0, 0x0F, 0x83, 0xFF, 0xFF,
0xC1, 0xF0, 0x0F, 0x07, 0xF0, 0x0F, 0xE0, 0xF0, 0x1C, 0x0F, 0x80, 0x01, 0xF0, 0x38, 0x1C, 0x1F,
0x87, 0xE1, 0xF8, 0x38, 0x00, 0x3E, 0x1F, 0xF8, 0x7C, 0x00, 0x00, 0x78, 0x3F, 0xFC, 0x1E, 0x00,
0x00, 0x70, 0x7C, 0x3E, 0x0E, 0x00, 0x00, 0x00, 0xF8, 0x1F, 0x00, 0x00, 0x00, 0x00, 0xF0, 0x0F,
0x00, 0x00, 0x00, 0x00, 0x61, 0x86, 0x00, 0x00, 0x00, 0x00, 0x03, 0xC0, 0x00, 0x00, 0x00, 0x00,
0x03, 0xC0, 0x00, 0x00, 0x00, 0x00, 0x01, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFE, 0x00, 0x07,
0xFF, 0x80, 0x00, 0xFF, 0x80, 0x07, 0xFF, 0x80, 0x00, 0xEB, 0xC0, 0x07, 0xFF, 0x80, 0x00, 0xE1,
0xE0, 0x07, 0x00, 0x00, 0x00, 0xE0, 0xE0, 0x07, 0x00, 0x00, 0x00, 0xE0, 0x40, 0x07, 0x00, 0x00,
0x00, 0xE0, 0x60, 0x07, 0x00, 0x00, 0x00, 0xE0, 0xE0, 0x07, 0x00, 0x00, 0x00, 0xE1, 0xE0, 0x07,
0xFC, 0x00, 0x00, 0xFF, 0xC0, 0x07, 0xFC, 0x00, 0x00, 0xF7, 0x80, 0x07, 0xFC, 0x00, 0x00, 0xFE,
0x00, 0x07, 0x00, 0x00, 0x00, 0xFC, 0x00, 0x07, 0x00, 0x00, 0x00, 0xF4, 0x00, 0x07, 0x00, 0x00,
0x00, 0xFE, 0x00, 0x07, 0x00, 0x00, 0x00, 0xEF, 0x00, 0x07, 0x00, 0x00, 0x00, 0xEF, 0x80, 0x07,
0x00, 0x00, 0x00, 0xE7, 0xC0, 0x07, 0x00, 0x00, 0x00, 0xE3, 0xE0, 0x07, 0x00, 0x00, 0x00, 0xE1,
0xE0, 0x07, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00

};

#define EEPROM_SIZE 512
#define NUM_SLOTS 10
#define SLOT_SIZE 50
#define NUM_LINES 4
#define NUM_SELECTIONS 4

#define RF_TRANSMIT_PIN D5
#define RF_RECEIVE_PIN D6

#define SCAN_TIME 5000

RCSwitch mySwitch = RCSwitch();

const int upButton = D0;
const int rightButton = D3;
const int downButton = D4;
const int leftButton = D7;

unsigned long lastDebounceTime = 0;
unsigned long debounceDelay = 50;

const int EEPROM_ADDRESSES[NUM_SLOTS] = {0, 50, 100, 150, 200, 250, 300, 350, 400, 450};
String EEPROM_VALUES[NUM_SLOTS] = {};

const String menuOptions[] = {"Record", "Playback", "Del EEPROM", "Raw Data"};
const String saveOptions[] = {"1", "2", "3", "4", "5", "6", "7", "8", "9", "10"};

int selection = 1;
int saveSelection = 1;

int actionMode = 0;
bool inRecord = false;
bool inPlayback = false;

bool debug = true;

void setup() {
  Serial.begin(115200);
  pinMode(upButton, INPUT_PULLUP);
  pinMode(downButton, INPUT_PULLUP);
  pinMode(rightButton, INPUT_PULLUP);
  pinMode(leftButton, INPUT_PULLUP);


  mySwitch.enableReceive(RF_RECEIVE_PIN);  // Receiver on interrupt 0 => that is pin #2
  mySwitch.enableTransmit(RF_TRANSMIT_PIN);

  if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }

  // Show initial display buffer contents on the screen --
  // the library initializes this with an Adafruit splash screen.
  displayLogo();
  
  // Clear the buffer
  display.clearDisplay();
  display.setTextSize(2);      // Normal 1:1 pixel scale
  display.setTextColor(SSD1306_WHITE); // Draw white text
  loadEEPROMValues();
  Serial.println("");
}

void displayLogo(){
  display.clearDisplay();
  display.drawBitmap(
    (display.width()  - LOGO_WIDTH ) / 2,
    (display.height() - LOGO_HEIGHT) / 2,
    logo_bmp, LOGO_WIDTH, LOGO_HEIGHT, 1);
  display.display();
  delay(2000);
}

void debugPrint(String toPrint) {
  if (debug) {
    Serial.println(toPrint);
  }
}

void clearEEPROM() {
  EEPROM.begin(EEPROM_SIZE);
  byte formatter = 255;
  for (int i = 0; i < EEPROM_SIZE; i++) {
    EEPROM.write(i, formatter);
    EEPROM.commit();
  }
  EEPROM.end();
}

void loadEEPROMValues() {
  EEPROM.begin(EEPROM_SIZE);
  for (int i = 0; i < NUM_SLOTS; i++) {
    String stringValue = "";
    byte first = EEPROM.read(EEPROM_ADDRESSES[i]);
    if (first == 255) {
      EEPROM_VALUES[i] = "";
    } else {
      for (int b = 0; b < SLOT_SIZE; b++) {
        byte readByte = EEPROM.read(EEPROM_ADDRESSES[i] + b);
        if (readByte != 255) {
          char readChar = EEPROM.read(EEPROM_ADDRESSES[i] + b);
          stringValue += readChar;
        } else {
          b = 255;
        }
      }
      EEPROM_VALUES[i] = stringValue;
      Serial.println("EEPROM Value at " + String(i + 1));
      Serial.println((String)EEPROM_VALUES[i]);
    }
  }
  EEPROM.end();
}
void recordToEEPROM(int slotNumber, String signalLength, String signalData) {
  EEPROM.begin(EEPROM_SIZE);
  String dataToRecord = signalLength + "." + signalData + "!";
  int dataLength = dataToRecord.length() + 1;
  char signalArray[dataLength];
  dataToRecord.toCharArray(signalArray, dataLength );
  for (int b = 0; b < dataLength; b++) {
    char signalChar = signalArray[b];
//    Serial.println(String(signalChar) + " saved to address " + String(EEPROM_ADDRESSES[slotNumber] + b));
    EEPROM.write(EEPROM_ADDRESSES[slotNumber] + b, signalChar);
    EEPROM.commit();

  }
  EEPROM.commit();
  EEPROM.end();

  loadEEPROMValues();
}

bool receiveRF() {
  bool capturedSignal = false;
  int startTime = millis();
  while (!capturedSignal) {
    if (mySwitch.available()) {
      String receivedValue = String(mySwitch.getReceivedValue());
      String bitLength = String(mySwitch.getReceivedBitlength());
      String pulseWidth = String(mySwitch.getReceivedDelay());
      delay(500);
      mySwitch.resetAvailable();

      recordToEEPROM(saveSelection-1, bitLength, receivedValue);
      capturedSignal = true;
      return true;
    }
    if (millis() - startTime > SCAN_TIME) {
      capturedSignal = true;
      return false;
    }
    if ((millis() - startTime) % 200 == 0) {
      delay(1);
    }
  }
}

void playSignal(String dataValue) {
  int bitLength = dataValue.substring(0, dataValue.indexOf(".")).toInt();
  int value = dataValue.substring((dataValue.indexOf(".") + 1), dataValue.indexOf("!")).toInt();
  delay(500);
  mySwitch.send(value, bitLength);
  delay(500);
}

bool readButtonPress(int buttonPin) {
  int reading = digitalRead(buttonPin);

  if (reading == HIGH) {
    return false;
  } else {
    delay(50);
    if (reading == digitalRead(buttonPin)) {
      delay(50);
      return true;
    }

  }
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

    case 3:
      inPlayback = true;
      clearEEPROMMode();
      break;
  }

}

void mainMenuScreen() {
  //display options
  display.setCursor(0, 0);
  display.clearDisplay();
  for (int i = 0; i < NUM_SELECTIONS; i++) {
    String text = menuOptions[i];
    display.setTextColor(SSD1306_WHITE);
    if (i + 1 == selection) {
      display.setTextColor(SSD1306_INVERSE);
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
    //if up pressed
    if (readButtonPress(upButton)) {
      if (selection == 1) {
        selection = NUM_SELECTIONS;
      } else {
        selection -= 1;
      }
      mainMenuScreen();
    }

    //if down pressed
    if (readButtonPress(downButton)) {
      if (selection == NUM_SELECTIONS) {
        selection = 1;
      } else {
        selection += 1;
      }
      mainMenuScreen();
    }

    //if left pressed
    if (readButtonPress(leftButton)) {
      //nothing
    }

    //if right pressed
    if (readButtonPress(rightButton)) {
      switch (selection) {
        case 1:
          actionMode = 1;
          break;
        case 2:
          actionMode = 2;
          break;
        case 3:
          actionMode = 3;
          break;
      }
    }
    delay(100);
  }
}

void saveSlotsScreen() {
  display.setCursor(0, 0);
  display.clearDisplay();
  int page = saveSelection / NUM_LINES;
  int remainder = saveSelection % NUM_LINES;
  if (remainder == 0) {
    page--;
  }
  int startingIndex = NUM_LINES * page;
  for (int i = startingIndex; i < NUM_SLOTS; i++) {
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
      if (readButtonPress(upButton)) {
        if (saveSelection == 1) {
          saveSelection = NUM_SLOTS;
        } else {
          saveSelection -= 1;
        }
        saveSlotsScreen();
      }

      //if down pressed
      if (readButtonPress(downButton)) {
        if (saveSelection == NUM_SLOTS) {
          saveSelection = 1;
        } else {
          saveSelection += 1;
        }
        saveSlotsScreen();
      }

      //if left pressed
      if (readButtonPress(leftButton)) {
        actionMode = 0;
        break;
      }

      //if right pressed
      if (readButtonPress(rightButton)) {
        curStep = 2;
      }
    }

    //display "Press right button to record"

    while (curStep == 2) {
      display.setCursor(0, 0);
      display.clearDisplay();
      display.println("Save Slot " + (String)saveSelection);
      display.display();
      if (readButtonPress(rightButton)) {
        curStep = 3;
      }
      if (readButtonPress(leftButton)) {
        curStep = 1;
      }
    }

    int millisTiming = 0;
    //record
    while (curStep == 3) {
      display.setCursor(0, 0);
      display.clearDisplay();
      display.println("Recording to " + (String)saveSelection);
      display.display();
      //record signals
      bool success = receiveRF();
      if (success) {
        curStep = 4;
      } else {
        display.setCursor(0, 0);
        display.clearDisplay();
        display.println("Failed To Detect Signal ");
        display.display();
        delay(2000);
        curStep = 2;
      }
    }


    //save to save slot
    while (curStep == 4) {
      display.setCursor(0, 0);
      display.clearDisplay();
      display.println("Saving signal to " + (String)saveSelection);
      display.display();
      delay(2000);
      curStep = 5;
    }

    while (curStep == 5) {
      //parse data

      display.setCursor(0, 0);
      display.clearDisplay();
      display.println("Press Left to go home");
      display.println("Press Right to save more");
      display.display();
      if (readButtonPress(rightButton)) {
        curStep = 1;
        Serial.println("Pressed right");
      }

      if (readButtonPress(leftButton)) {
        Serial.println("Pressed left");
        actionMode = 0;
        break;
      }
    }
  }
}

void playbackMode() {
  int curStep = 1;
  while (actionMode == 2) {

    //Select Save
    while (curStep == 1) {
      saveSlotsScreen();
      //if up pressed
      if (readButtonPress(upButton)) {
        if (saveSelection == 1) {
          saveSelection = NUM_SLOTS;
        } else {
          saveSelection -= 1;
        }
        saveSlotsScreen();
      }

      //if down pressed
      if (readButtonPress(downButton)) {
        if (saveSelection == NUM_SLOTS) {
          saveSelection = 1;
        } else {
          saveSelection += 1;
        }
        saveSlotsScreen();
      }

      //if left pressed
      if (readButtonPress(leftButton)) {
        actionMode = 0;
        break;
      }

      //if right pressed
      if (readButtonPress(rightButton)) {
        curStep = 2;
      }
    }

    // Playback Save
    String signalData = EEPROM_VALUES[saveSelection - 1];
    while (curStep == 2) {
      if (signalData == "") {
        display.setCursor(0, 0);
        display.clearDisplay();
        display.println("No Signal in " + (String)saveSelection);
        display.display();
        delay(2000);
        curStep = 1;
      } else {
        display.setCursor(0, 0);
        display.clearDisplay();
        display.println("Play Signal " + (String)saveSelection + "?");
        display.println("Press Left to go back");
        display.display();
        //if left pressed
        if (readButtonPress(leftButton)) {
          curStep = 1;
        }

        //if right pressed
        if (readButtonPress(rightButton)) {
          //playback saved signal

          curStep = 3;
        }
      }

    }


    while (curStep == 3) {
      display.setCursor(0, 0);
      display.clearDisplay();
      display.println("Playing Signal  " + (String)saveSelection);
      display.display();
      delay(500);
      String signalData = EEPROM_VALUES[saveSelection - 1];
      display.setCursor(0, 0);
      display.clearDisplay();
      display.println(signalData);
      display.display();
      delay(500);
      playSignal(signalData);
      delay(5 00);
      curStep = 4;
    }

    while (curStep == 4) {
      display.setCursor(0, 0);
      display.clearDisplay();
      display.println("Play Signal  " + (String)saveSelection + " again?");
      display.println("Press Left to go back");
      display.display();
      if (readButtonPress(leftButton)) {
        curStep = 1;
      }

      //if right pressed
      if (readButtonPress(rightButton)) {
        //playback saved signal
        curStep = 3;
      }
    }
  }
}


void clearEEPROMMode() {
  int curStep = 1;
  while (actionMode == 3) {

    //Select Save
    while (curStep == 1) {
      display.setCursor(0, 0);
      display.clearDisplay();
      display.println("Press Right");
      display.display();
      //if up pressed
      if (readButtonPress(upButton)) {
        actionMode = 0;
        break;
      }

      //if down pressed
      if (readButtonPress(downButton)) {
        actionMode = 0;
        break;
      }

      //if left pressed
      if (readButtonPress(leftButton)) {
        actionMode = 0;
        break;
      }

      //if right pressed
      if (readButtonPress(rightButton)) {
        curStep = 2;
      }
    }

    while (curStep == 2) {
      display.setCursor(0, 0);
      display.clearDisplay();
      display.println("Press Left");
      display.display();
      //if up pressed
      if (readButtonPress(upButton)) {
        actionMode = 0;
        break;
      }

      //if down pressed
      if (readButtonPress(downButton)) {
        actionMode = 0;
        break;
      }

      //if left pressed
      if (readButtonPress(leftButton)) {
        curStep = 3;
      }

      //if right pressed
      if (readButtonPress(rightButton)) {
        actionMode = 0;
        break;
      }
    }

    while (curStep == 3) {
      display.setCursor(0, 0);
      display.clearDisplay();
      display.println("Press Up");
      display.display();
      //if up pressed
      if (readButtonPress(upButton)) {
        curStep = 4;
      }

      //if down pressed
      if (readButtonPress(downButton)) {
        actionMode = 0;
        break;
      }

      //if left pressed
      if (readButtonPress(leftButton)) {
        actionMode = 0;
        break;
      }

      //if right pressed
      if (readButtonPress(rightButton)) {
        actionMode = 0;
        break;
      }
    }

    while (curStep == 4) {
      display.setCursor(0, 0);
      display.clearDisplay();
      display.println("Press Down");
      display.display();
      //if up pressed
      if (readButtonPress(upButton)) {
        actionMode = 0;
        break;
      }

      //if down pressed
      if (readButtonPress(downButton)) {
        curStep = 5;
      }

      //if left pressed
      if (readButtonPress(leftButton)) {
        actionMode = 0;
        break;
      }

      //if right pressed
      if (readButtonPress(rightButton)) {
        actionMode = 0;
        break;
      }
    }

    while (curStep == 5) {
      display.setCursor(0, 0);
      display.clearDisplay();
      display.println("Clear EEPROM with Right");
      display.display();
      //if up pressed
      if (readButtonPress(upButton)) {
        actionMode = 0;
        break;
      }

      //if down pressed
      if (readButtonPress(downButton)) {
        actionMode = 0;
        break;
      }

      //if left pressed
      if (readButtonPress(leftButton)) {
        actionMode = 0;
        break;
      }

      //if right pressed
      if (readButtonPress(rightButton)) {
        display.setCursor(0, 0);
        display.clearDisplay();
        display.println("Clearing EEPROM");
        display.display();
        delay(1500);
        clearEEPROM();
        actionMode = 0;
        break;
      }
    }
  }
}
