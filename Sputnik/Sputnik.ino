/*
  Chris Luginbuhl
  Mar 28, 2018
  Experiences and Interfaces
  This is the code to run the Sputnik radar detector-driven beep
*/

#include <Adafruit_CircuitPlayground.h>

const int toneDuration = 250;  //duration in milliseconds
const int toneFreq = 800;      //frequency in Hz
const int analogInPin = A1;    // Analog input pin that radar VOut pin is attached to
const int analogPotPin = A2;
const int numSamples = 50;
const int minObserved = 240;
const int maxObserved = 500;


long timer = 0;
long beepTimer = 0;
long sensorSum = 0;
int beepDelay = 4000;
int movement = 0;
int sensorValue = 0;        // analog value read from the radar 0-1023

void setup() {

  Serial.begin(9600);
  CircuitPlayground.begin();

}

uint8_t effect = 1;


void loop() {
  sensorSum = 0;
  for (int i = 0; i < numSamples; i++) {
    sensorSum += analogRead(analogInPin);
    delay(3); //let sensor settle after read
  }
  sensorValue = sensorSum / numSamples - analogRead(analogPotPin);         //note this is integer division. Should be fine with these values.
  movement = map(sensorValue, minObserved, maxObserved, 0, 100);  //empirically determined upper and lower bounds mapped to a percent scale
  movement = constrain(movement, 0, 100);         //clip anything outside this range
  //  movement = (2*movement + pow(movement, 2) / 100) / 3; //inverse bell curve to make beeping less frequent without reducing possibility to go to 100%
  beepDelay = map(movement, 0, 100, 4000, 250);

  if ((millis() - beepTimer) > beepDelay) {
    if (CircuitPlayground.slideSwitch()) {
      CircuitPlayground.playTone(toneFreq, toneDuration, false); //false is a flag to set non-blocking behaviour
    }
    beepTimer = millis();
  }

  // print the results to the Serial Monitor:
  Serial.print("sensor = ");
  Serial.print(sensorValue);
  Serial.print("\t pot = ");
  Serial.print(analogRead(analogPotPin));
  Serial.print("\t output = ");
  Serial.print(movement);
  Serial.print("\t beepDelay = ");
  Serial.println(beepDelay);
}
