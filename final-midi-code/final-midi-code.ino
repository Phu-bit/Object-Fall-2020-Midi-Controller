//resources for this code:
//https://www.youtube.com/watch?v=0L7WAMFWSgY&list=PL4_gPbvyebyH2xfPXePHtx8gK5zPBrVkg
//https://www.youtube.com/watch?v=JZ5yPdoPooU&t=1s
//https://www.youtube.com/watch?v=kESotTqQgm4

#include <MIDI.h>
MIDI_CREATE_DEFAULT_INSTANCE();
//buttons
const int numButtons = 12; //*  total numbers of buttons
const int buttonPinArray[numButtons] = {2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13};  
// potentiometers
const int numPotentiometers = 5; // total numbers of pots
const int potPinArray[numPotentiometers] = {A0, A1, A5, A3, A4}; //* pins of each pot


int buttonCState[numButtons] = {};        // stores the button current value
int buttonPState[numButtons] = {};        // stores the button previous value

byte pin13index = 12; //wyat array index is pin 13

// debounce
unsigned long lastDebounceTime[numButtons] = {0};
unsigned long debounceDelay = 5;


int potCState[numPotentiometers] = {0}; // Current state of the pot
int potPState[numPotentiometers] = {0}; // Previous state of the pot
int potVar = 0; // Difference between the current and previous state of the pot

int midiCState[numPotentiometers] = {0}; // Current state of the midi value
int midiPState[numPotentiometers] = {0}; // Previous state of the midi value

const int TIMEOUT = 300; //* Amount of time the potentiometer will be read after it exceeds the varThreshold
const int varThreshold = 10; //* Threshold for the potentiometer signal variation
boolean potMoving = true; // If the potentiometer is moving
unsigned long PTime[numPotentiometers] = {0}; // Previously stored time
unsigned long timer[numPotentiometers] = {0}; // Stores the time that has elapsed since the timer was reset


byte midiCh = 1; //* MIDI channel to be used
byte note = 36; //* Lowest note to be used
byte cc = 1; //* Lowest MIDI CC to be used

void setup() {

  // 31250 for MIDI class compliant
  Serial.begin(31250);

  // Buttons
  // Initialize buttons with pull up resistors
  for (int i = 0; i < numButtons; i++) {
    pinMode(buttonPinArray[i], INPUT_PULLUP);
  }

  pinMode(buttonPinArray[pin13index], INPUT); // pin 13 automatically has a pulldown resistor 

}

void loop() {

  buttons();
  potentiometers();

}


void buttons() {

  for (int i = 0; i < numButtons; i++) {

    buttonCState[i] = digitalRead(buttonPinArray[i]);  // read pins from arduino

    if (i == pin13index) {
      buttonCState[i] = !buttonCState[i]; // inverts the pin 13 because it has a pull down resistor instead of a pull up
    }

    if ((millis() - lastDebounceTime[i]) > debounceDelay) { //debounce

      if (buttonPState[i] != buttonCState[i]) {
        lastDebounceTime[i] = millis();

        if (buttonCState[i] == LOW) {

          // Sends the MIDI note ON

          MIDI.sendNoteOn(note + i, 127, midiCh); // note, velocity, channel

        }
        else {


          MIDI.sendNoteOn(note + i, 0, midiCh); // note, velocity, channel

        }
        buttonPState[i] = buttonCState[i];
      }
    }
  }
}


void potentiometers() {


  for (int i = 0; i < numPotentiometers; i++) { // Loops through all the potentiometers

    potCState[i] = analogRead(potPinArray[i]); // reads the pins from arduino

    midiCState[i] = map(potCState[i], 0, 1023, 0, 127); // Maps the reading of the potCState to a value usable in midi

    potVar = abs(potCState[i] - potPState[i]); // Calculates the absolute value between the difference between the current and previous state of the pot

    if (potVar > varThreshold) { // Opens the gate if the potentiometer variation is greater than the threshold
      PTime[i] = millis(); // Stores the previous time
    }

    timer[i] = millis() - PTime[i]; // Resets the timer 11000 - 11000 = 0ms

    if (timer[i] < TIMEOUT) { // If the timer is less than the maximum allowed time it means that the potentiometer is still moving
      potMoving = true;
    }
    else {
      potMoving = false;
    }

    if (potMoving == true) { // If the potentiometer is still moving, send the change control
      if (midiPState[i] != midiCState[i]) {


        MIDI.sendControlChange(cc + i, midiCState[i], midiCh); // cc number, cc value, midi channel


        potPState[i] = potCState[i]; // Stores the current reading of the potentiometer to compare with the next
        midiPState[i] = midiCState[i];
      }
    }
  }
}
