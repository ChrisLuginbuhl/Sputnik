/*
  Chris Luginbuhl
  Mar 28, 2018
  Experiences and Interfaces
  This is the code to run the Sputnik radar detector-driven beep
*/

#include <Adafruit_CircuitPlayground.h>
#define SAMPLE_WINDOW   5  // Sample window for average level  - 5ms means 200Hz sound is the lowest we'll measure
#define SOUND_FLOOR     59  // Lower range of mic sensitivity in dB SPL
#define SOUND_CEILING  110  // Upper range of mic sensitivity in db SPL
#define SOUND_THRESHOLD 100
#define NUM_PEAKS 20

const int toneDuration = 250;  //duration in milliseconds
const int toneFreq = 800;      //frequency in Hz
const int analogInPin = A1;    // Analog input pin that radar VOut pin is attached to
const int analogPotPin = A2;
const int numSamples = 50;
const int minObserved = 240;
const int maxObserved = 500;


int soundPeak[] = {
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
}; //20 zeroes
int peakIndex = 0;
long timer = 0;
long beepTimer = 0;
long sensorSum = 0;
int beepDelay = 4000;
int movement = 0;
int sensorValue = 0;        // analog value read from the radar 0-1023
int numPixels = 10;
uint32_t color = 0;
bool mute = false;
int muteTimeout = 0;

void setup() {

  Serial.begin(9600);
  CircuitPlayground.begin();
  CircuitPlayground.setBrightness(255);
  CircuitPlayground.clearPixels();
  numPixels = CircuitPlayground.strip.numPixels();
}

uint8_t effect = 1;


void loop() {
  
  int peakToPeak = 0;
  float peakToPeakSum = 0;
 
  sensorSum = 0;

  //read from radar and mic (with average for smoothing)
  for (int i = 0; i < numSamples; i++) {
    sensorSum += analogRead(analogInPin);
    peakToPeakSum += CircuitPlayground.mic.soundPressureLevel(SAMPLE_WINDOW);
  }


  sensorValue = sensorSum / numSamples - analogRead(analogPotPin);         //note this is integer division. Should be fine with these values.
  movement = map(sensorValue, minObserved, maxObserved, 0, 100);  //empirically determined upper and lower bounds mapped to a percent scale
  movement = constrain(movement, 0, 100);         //clip anything outside this range
  //  movement = (2*movement + pow(movement, 2) / 100) / 3; //inverse bell curve to make beeping less frequent without reducing the
  //possibility to go to 100%
  beepDelay = map(movement, 0, 100, 4000, 250);  //sets max and min time between beeps

  //"play back" (with LEDs) the last sound exceeding the threshold

  if (soundPeak[peakIndex + 1] > (SOUND_FLOOR + 3) && (soundPeak[peakIndex + 1] <= (SOUND_FLOOR + 10))) {
    color = CircuitPlayground.strip.Color(255, 255, 175); // whitish light - contrasting slightly with blue interior of sputnik.
  } else if (soundPeak[peakIndex + 1] > (SOUND_FLOOR + 20)) {
    color = CircuitPlayground.strip.Color(255, 40, 10);
  } else {
    color = CircuitPlayground.strip.Color(0, 0, 0);
  }
  for (int i = 0; i < numPixels; i++) {
    CircuitPlayground.strip.setPixelColor(i, color);
  }
  CircuitPlayground.strip.show();


  //Cycle to the next spot on the array, wrapping around if at the end
  if (peakIndex < (NUM_PEAKS - 2)) {
    peakIndex++;
  } else {
    peakIndex = 0;
  }

  peakToPeak = int(peakToPeakSum / numSamples);
  soundPeak[peakIndex] = peakToPeak; //storing the last 20 values in an array in order to "play them back" with LEDs
 
  if (peakToPeak > (SOUND_FLOOR + 10))
  {
    Serial.println("Mute!");
    mute = true;
    muteTimeout = millis();
  } else if ((millis() - muteTimeout) > 15000) {
    mute = false;
    Serial.println("....unmute!");
  }

  if ((millis() - beepTimer) > beepDelay) {
    if (CircuitPlayground.slideSwitch() && !mute) {
      CircuitPlayground.playTone(toneFreq, toneDuration, true); //true sets blocking behaviour - don't want beep to register in mic
    }
    beepTimer = millis();
  }



  // print the results to the Serial Monitor:
  Serial.print("Sound: ");
  Serial.print(peakToPeak);
  Serial.print("\t motion sensor = ");
  Serial.print(sensorValue);
  Serial.print("\t pot = ");
  Serial.print(analogRead(analogPotPin));
  Serial.print("\t output = ");
  Serial.print(movement);
  Serial.print("\t beepDelay = ");
  Serial.println(beepDelay);
  for (int i = 0; i < 20; i++) {
    Serial.print(soundPeak[i]);
    Serial.print(" ");
  }
  Serial.println();


}
