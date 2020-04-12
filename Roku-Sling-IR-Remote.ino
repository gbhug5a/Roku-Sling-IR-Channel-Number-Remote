
/*  Roku-Sling IR Channel Number Remote v3.0
    Arduino Pro Mini - 3.3V, 8MHz */

#include <Keypad.h>
#include <IRremote.h>
#include <avr/sleep.h>
#include <avr/wdt.h>

IRsend irsend;

const int totChans = 24;        // total number of channels in My Channels ("MC")
const int maxMoves = 12;        // if Moves greater than this, go the other way.
const byte Channel [totChans]={35,48,29,47,34,45,39,28,55,148,31,56,46,60,58,59,30,129,41,52,49,70,72,71};
int  currentChan = 0;           // the position of the current playing channel on the MC line
int  targetChan = 0;            // the position of the target channel on the MC line
int Moves = 0;                  // # of RiGHT or LEFT moves to the new channel
const int Interval = 500;       // interval between IR commands, in milliseconds

const unsigned long HOME = 0x5743C03F;    // the Roku IR codes for these buttons
const unsigned long BACK = 0x57436699;
const unsigned long UP = 0x57439867;
const unsigned long DOWN = 0x5743CC33;
const unsigned long LEFT = 0x57437887;
const unsigned long RIGHT = 0x5743B44B;
const unsigned long OK = 0x574354AB;

const unsigned long TVPOWER = 0x41A2;     // the Sharp TV IR codes
const unsigned long TVINPUT = 0x4322;
const unsigned long TVUP = 0x43AA;
const unsigned long TVENTER = 0x43BE;

unsigned long Direction = 0;        // will equal RIGHT or LEFT, depending

const byte ROWS = 4;                //four rows
const byte COLS = 3;                //three columns
byte rowPins[ROWS] = {4, 5, 6, 7};  //row pins of the keypad - all Port D - don't change
byte colPins[COLS] = {2, 8, 9};     //column pins of the keypad
byte PCIenable = 4;                 //pin change interrupt for Port D - for PCICR register
byte PCImask = 0xF0;                //bits 4, 5, 6, and 7 of Port D - for PCMSK2 register

const char keys[ROWS][COLS] = {
  {'1','2','3'},
  {'4','5','6'},
  {'7','8','9'},
  {'*','0','#'}
};

Keypad keypad = Keypad( makeKeymap(keys), rowPins, colPins, ROWS, COLS );

int channelNum = 0;                   // the number keyed in to the keypad
unsigned long keyTime = 0;            // millis value when last key pressed
int timeLimit = 1000;                 // assume no more keys after this
int i,c,r = 0;
char key, keyCopy;
bool dowhile = true;

void setup(){

  for (i = 2; i < 20; i++) {          // all pins to one rail or the other - power saving
    pinMode(i,OUTPUT);
  }
  for (r = 0; r < ROWS; r++) {        // the ROW pins will generate pin change interrupts
    pinMode(rowPins[r],INPUT_PULLUP); //   to wake the MCU, then sense key press in getKey()
  }
  ADCSRA = 0;                         // disable ADC for power saving
  wdt_disable();                      // disable WDT for power saving

  PCICR = PCICR | PCIenable;          // enable pin change interrupts, but mask still empty
}

void loop(){

  for (c = 0; c < COLS; c++) {
    pinMode(colPins[c],OUTPUT);
    digitalWrite(colPins[c], LOW);    // Set all columns low - for pin change interrupts
  }
  PCMSK2 = PCMSK2 | PCImask;          // enable interrupt mask bits for the ROW pins

  set_sleep_mode (SLEEP_MODE_PWR_DOWN); // Deep sleep
  sleep_enable();
  sleep_bod_disable();                // disable brownout detector during sleep
  sleep_cpu();                        // now go to sleep

  //  *WAKE UP* from pressing any key

  for (c = 0; c < COLS; c++) {        // all columns back to INPUT - for getKey function
    pinMode(colPins[c],INPUT);        //   getKey takes one at a time OUTPUT LOW
  }

  channelNum = 0;
  keyTime = millis();                 // in case no key actually pressed, or too short
  keyCopy = 0;
  dowhile = true;

  while(dowhile) {                    // look until timeout, then process, then sleep

    key = keypad.getKey();            // get key currently pressed, if any

    if (key) {
      keyTime = millis();                             // mark time when key pressed
      keyCopy = key;
      if ((key >= '0') && (key <= '9')) {
        int ikey = key;                               // convert character to integer
        channelNum = (channelNum * 10) + ikey - 48;   // if another key, shift left & add
      }
    }
    else {
      if ((millis() - keyTime) > timeLimit) {         // time has expired
        if (keyCopy == '*') {
          if (channelNum == 0) {
            currentChan = 0;
            sendRemote(HOME);
            delay(5000);
            sendTV(TVPOWER);
          }
          else {                    //if channel number entered before *
            sendRemote(HOME);       //Roku home
            delay(5000);
            sendRemote(RIGHT);      //right to Sling
            sendRemote(OK);         //select Sling
            delay(15000);
            sendRemote(DOWN);       //down to My Channels line
            currentChan = 0;        //entry point is FX
            calculate();            //calculate and perform steps to new channel
          }
        }
        else if (keyCopy == '#') {
          if (channelNum == 0) {
            sendTV(TVINPUT);
            sendTV(TVUP);
            sendTV(TVENTER);
          }
          else {
            targetChan = currentChan;
            for(i = 0; i < totChans; ++i) {
              if (Channel[i] == channelNum) targetChan = i;
            }
            if (targetChan != currentChan) currentChan = targetChan;
          }
        }
        else {
          calculate();              //process the channel number, and reset values
        }
        dowhile = false;            //end the While loop, go to sleep
      }
    }
  }                                 //While
}                                   //Loop

void calculate() {
  targetChan = currentChan;
  for(i = 0; i < totChans; ++i) {
    if (Channel[i] == channelNum) targetChan = i;
  }
  if (targetChan > currentChan) {
    Moves = targetChan - currentChan;
    Direction = RIGHT;
    if (Moves > maxMoves) {         // if too many moves, go the other way
      Moves = totChans +1 - Moves;
      Direction = LEFT;
    }
  }
  if (currentChan > targetChan) {
    Moves = currentChan - targetChan;
    Direction = LEFT;
    if (Moves > maxMoves) {
      Moves = totChans +1 - Moves;
      Direction = RIGHT;
    }
  }

  if (currentChan != targetChan) {
    if (keyCopy != '*') {           //if already watching a channel, get back to MC
      sendRemote(BACK);
      delay(Interval);
//      sendRemote(BACK);
      delay(Interval);
    }
    for (i=0; i<Moves; i++) {       //perform the scrolling steps
      sendRemote(Direction);
    }
  }
  if (( currentChan != targetChan)  || (keyCopy == '*')) sendRemote(OK);
  currentChan = targetChan;
}

void sendRemote(unsigned long Output) {
  irsend.sendNEC(Output,32);
  delay(Interval);
}
void sendTV(unsigned long Output) {
  irsend.sendSharpRaw(Output,15);
  delay(Interval);
}
ISR (PCINT2_vect) {                 // pin change interrupt service routine
  PCMSK2 = PCMSK2 & (~PCImask);     // clear mask bits = disable further interrupts
  PCIFR = PCIFR | PCIenable;        // clear any pending interrupt flag bits
}
