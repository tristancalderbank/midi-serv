/*
midi-serv

Author: Tristan Caldebank
Date: April 4, 
*/

#include <Servo.h>
#include <MIDI.h>

#define PROGRAM_TIMESTEP 1   // (ms) How long each program timestep takes

#define MIDI_CHANNEL_START 9 // Midi channels count up from this number up to max channel of (MIDI_CHANNEL_START + NUM_CHANNELS)
#define NUM_CHANNELS 6

#define MIDI_NOTE_OFFSET 28  // Added to incoming notes numbers, used to adjust note/angle mapping

#define START_POS 110        // (degrees)
#define START_TICK_DELAY 1   // (ms * PROGRAM_TIMESTEP) How long a 1 degree servo movement takes, controlled by velocity value
#define TIMER_STOPPED -1     // -1 timer value means timer disabled

#define DEBUG_PIN 13

MIDI_CREATE_DEFAULT_INSTANCE();

byte PIN_MAP[NUM_CHANNELS] = {3, 5, 9, 2, 4, 8}; // Assign midi channels to pins, index 0 corresponds to channel MIDI_CHANNEL_START

Servo servo[NUM_CHANNELS];

// These are important

byte pos[NUM_CHANNELS];       // Current servo position
byte nextPos[NUM_CHANNELS];   // Next servo position
byte tickDelay[NUM_CHANNELS]; // How long each 1 degree movement takes
byte timer[NUM_CHANNELS];     // Timer which counts down from tickDelay to 0 for each 1 degree movement
 
// Advances the program by one time step (one tick == 1ms)
// Moves a servo 1 degree towards nextPos for every timer cycle completed
void moveTick() {

    byte i;
    
    for (i = 0; i < NUM_CHANNELS; i++) {
      
        // If reached nextPos, disable timer 
        
        if (pos[i] == nextPos[i]) {
          timer[i] = -1;
        }
      
        // If more ticks left, decrement timer by 1
      
        if (timer[i] > 0) {
          timer[i]--;
        }
        // If timer completed, move the servo one degree towards nextPos
  
        else if (timer[i] == 0) {
          pos[i] += (nextPos[i] > pos[i]) - (nextPos[i] < pos[i]);
          servo[i].write(pos[i]);        
          timer[i] = tickDelay[i];
        }
    }
}

void handleNoteOn(byte channel, byte note, byte velocity) {

  byte i;

  // Check its in our channel range
  if (channel >= MIDI_CHANNEL_START && channel <= (MIDI_CHANNEL_START + NUM_CHANNELS)) {
    
      i = channel - MIDI_CHANNEL_START;           // Calculate actual array index

      nextPos[i] = note + MIDI_NOTE_OFFSET + 20;  // Set target position
      tickDelay[i] = velocity / 10 + 1;           // Set how long each movement tick will take
      timer[i] = tickDelay[i];                    // Load/start the timer with the delay
    
  }

  digitalWrite(DEBUG_PIN, HIGH); // Debug blink on any MIDI message

}

void handleNoteOff(byte channel, byte note, byte velocity) {
  digitalWrite(DEBUG_PIN, LOW); // Debug blink on any MIDI message
}

void initMidi() {
  MIDI.begin(MIDI_CHANNEL_OMNI);
  MIDI.setHandleNoteOn(handleNoteOn);
  MIDI.setHandleNoteOff(handleNoteOff);
}

void initServo() { 

  byte i;

  for (i = 0; i < NUM_CHANNELS; i++) {
    
    timer[i] = TIMER_STOPPED;
    pos[i] = START_POS;
    nextPos[i] = START_POS;
    tickDelay[i] = START_TICK_DELAY;

    servo[i].attach(PIN_MAP[i]); 
    servo[i].write(pos[i]);

  }
}

void setup() {
  initServo();
  initMidi();
}

void loop() {
  MIDI.read();
  moveTick();
  delay(PROGRAM_TIMESTEP); 
}


