/*****************************************************************************************
 * Project Hector:                                                                       *
 *                                                                                       * 
 * A small car chassis has a pair of light sensors.  The car will follow the light and   *
 * try to extricate itself from the darkness.                                            *
 *                                                                                       *
 *                                                                            AJC / 2017 *
 *****************************************************************************************/
#include <AFMotor.h> // Adafruit Motor Shield v1 Library

/************************************************************
 *            Global Variables and Constants                *
 ************************************************************/
// Digital assignments for the pins the shield
// does not use directly
const int A0Pin = 14;
const int A1Pin = 15;
const int A2Pin = 16;
const int A3Pin = 17;
const int A4Pin = 18;
const int A5Pin = 19;
const int LEDPin = 13;

// Analog pin assignments (for the light sensors)
const int A0Analog = 0;
const int A1Analog = 1;

// Stores the average light state from startup
int lightaverage = 0;

// Counter to blink the LED.
int ledcounter = 0;


/************************************************************
 *            Hardware and Firmware Initialization          *
 ************************************************************/

// Set up the latches and such for the motors.
AF_DCMotor motor1(3, MOTOR12_1KHZ);  
AF_DCMotor motor2(4, MOTOR12_1KHZ);

void setup() {
  
  Serial.begin(9600);
  Serial.print("Hector: Initialize\n");
  Serial.print("----------------------------------------\n");

  // All unused analog input pins should be set digital
  // and pulled up
  Serial.print("  Setting Digital Pins\n");
  pinMode(A2Pin, INPUT_PULLUP);
  pinMode(A3Pin, INPUT_PULLUP);
  pinMode(A4Pin, INPUT_PULLUP);
  pinMode(A5Pin, INPUT_PULLUP);

  // Our LED
  pinMode(LEDPin, OUTPUT);
  
  // Make sure our motors stay put
  Serial.print("  Resetting Motors\n");
  motor1.setSpeed(0);
  motor2.setSpeed(0);

  Serial.print("  Measuring Ambient Light\n");

  int l[5];
  int r[5];

  // Run a quick loop, measure and average the lighting before 
  // we start running
  for (int a = 0; a < 5; a++) {
    r[a] = 1023 - analogRead(0);
    l[a] = 1023 - analogRead(1);
  }

  // We use this average later
  lightaverage = (r[0] + r[1] + r[2] + r[3] + r[4] + l[0] + l[1] + l[2] + l[3] + l[4]) / 10;
  
  // I save program space and ~80 bytes of RAM by not 
  // using sprintf. 
  Serial.print("    Average of all readings:\t");
  Serial.print(lightaverage);
  Serial.print("\n");

  Serial.print("----------------------------------------\n");  
  Serial.print("Hector: Ready\n");

}

/************************************************************
 *                     Subroutines                          *
 ************************************************************/

/**************************************************
 * setmotorspeeds - Sets both motor speeds.       *
 **************************************************/
void setmotorspeeds(int m1, int m2) {

/*  
  Serial.print("NOTICE: Motor speeds (");
  Serial.print(m1);
  Serial.print("/");
  Serial.print(m2);
  Serial.print(")\n");
*/

  motor1.setSpeed(m1);
  motor2.setSpeed(m2);
  
}

/************************************************************
 *                  Main Execution Loop                     *
 ************************************************************/
void loop() {

  int motor1speed = 0;                 
  int motor2speed = 0;

  // Take our readings this go-round.
  int reading1 = 1023 - analogRead(A0Analog);
  int reading2 = 1023 - analogRead(A1Analog);
  int average = (reading1 + reading2) / 2;
  int difference = reading1 - reading2;

/*
  Serial.print("NOTICE: Light readings (");
  Serial.print(reading1);
  Serial.print("/");
  Serial.print(reading2);
  Serial.print(") Difference: ");
  Serial.print(difference);
  Serial.print("\n");
*/
  
  // Handle unexpected darkness by pausing, turning, and backing up  
  if (reading1 < lightaverage - 200 && reading2 < lightaverage - 200) {
    
    setmotorspeeds(0,0);
    
    delay(500);

    // We use what we can sense to determine which way we turn
    if ( difference > 0) {
      motor1.run(BACKWARD);
      motor2.run(FORWARD);
    } else {
      motor1.run(FORWARD);
      motor2.run(BACKWARD);
    }

    // Turn and run away quickly
    setmotorspeeds(255,255);
    delay(500);
    motor1.run(FORWARD);
    motor2.run(FORWARD);  
    delay(500);

    // Let the regular routine set our speed again
    setmotorspeeds(0,0);
    
  } else {

    
    // We want a graduated response to the presence of
    // uniform extra light.  The stronger the light,
    // the faster we move toward it!
    if ( (difference > -60) && (difference < 60) ) {

      motor2speed = 64;
      motor1speed = 64;
 
      if ( abs(average - lightaverage) > 100 ) {
        motor2speed = 128;
        motor1speed = 128;
      } 

      if ( abs(average - lightaverage) > 200 ) {
        motor2speed = 192;
        motor1speed = 192;
      }

      // TALLY HO
      if ( abs(average - lightaverage) > 300 ) {
        motor2speed = 255;
        motor1speed = 255;
      }

      // If we have this much light, stop and bask in it
      // This will never happen in a bright lighting situation
      if ( abs(average - lightaverage) > 400 ) {
        motor2speed = 0;
        motor1speed = 0;
      }
      

    // If the light presence isn't uniform, 
    // turn toward the light
    } else {
      if (difference < 0) {
        motor1speed = 64;
        motor2speed = 192;
      } else {
        motor2speed = 64;
        motor1speed = 192;
      }
      
    }

    // Apply our new motor speeds and gogoGO
    setmotorspeeds(motor1speed, motor2speed);

/*
    Serial.print("NOTICE: LED Counter (");
    Serial.print(ledcounter);
    Serial.print(")\n");
*/

    motor1.run(FORWARD);
    motor2.run(FORWARD);  
    
  }

    // Blink the lone built-in Arduino peripheral balefully
    ledcounter++;
    if (ledcounter > 1000) {
      digitalWrite(LEDPin, HIGH);
    } 
    if (ledcounter > 2000) {
      digitalWrite(LEDPin, LOW);
      ledcounter = 0;
    }

}
