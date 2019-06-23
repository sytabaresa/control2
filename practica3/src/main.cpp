/**
 * GENERAL SYSTEMS CONTROL PROGRAM
 * By: Sebastian Yesid Tabares Amaya
 * Licence: GPLv3
 */

#include <Arduino.h>
#include "Encoder.h"

// Encoder encoder(2, 4);

// analog input in
#define CURRENTpin A6
#define SENSORpin A0

#define INpin A2
#define STOPpin 12
#define BUTTONpin 11

#define ENpin 7
#define PWMpin 9
#define OUTApin 5
#define OUTBpin 4

#define K .1
#define ZERO 511
#define DEAD 0.6
#define MIN 7
#define LIM 100

#define CUERPO double

uint16_t in, in2 = 0;
CUERPO in1, ref, error, pos, out = 0;

template <typename type>
type sign(type value)
{
  return type((value > 0) - (value < 0));
}

/**
 * SETUP 
 */
void setup()
{
  // encoder pin on interrupt 0 (pin 2)
  attachInterrupt(0, doEncoderA, CHANGE);

  // encoder pin on interrupt 1 (pin 3)
  attachInterrupt(1, doEncoderB, CHANGE);

  setEncoder(2, 3);

#ifdef DEBUG
  // Serial begin
  Serial.begin(115200);
  Serial.setTimeout(1500);
#endif

  // analog input config
  pinMode(SENSORpin, INPUT);
  pinMode(CURRENTpin, INPUT);
  pinMode(INpin, INPUT);
  pinMode(STOPpin, INPUT_PULLUP);
  pinMode(BUTTONpin, INPUT_PULLUP);

  pinMode(ENpin, OUTPUT);
  pinMode(PWMpin, OUTPUT);
  pinMode(OUTApin, OUTPUT);
  pinMode(OUTBpin, OUTPUT);

  digitalWrite(ENpin, HIGH);
}

void output(CUERPO value)
{
  value += sign(value)*MIN;
  value = value > LIM ? LIM : value;
  value = value < -LIM ? -LIM : value;
  value = abs(value) < MIN+DEAD ? 0: value;

  digitalWrite(OUTApin, value > 0);
  digitalWrite(OUTBpin, value < 0);

  analogWrite(PWMpin, map(abs(value), 0, LIM, 0, 255));
}

#define DES 40
void resetPosition()
{
  ref = 0;
  position > 0 ? output(-DES) : output(DES);
}

void smartDelay(unsigned long us)
{
  unsigned long fin = micros() + us;
  do
  {
    if (!digitalRead(BUTTONpin))
    {
      resetPosition();
    }
    if (!digitalRead(STOPpin))
    {
      output(0);
      out = 0;
    }
    pos = (CUERPO)map(position, 0, DISTANCE, 0, 1024);
    rotating = true;

#ifdef DEBUG
    // double t = in * 0.02434;
    // Serial.print(in);
    // Serial.print(",");
    // Serial.print(in1);
    // Serial.print(",");
    Serial.print(ref);
    Serial.print(",");
    Serial.print(error);
    Serial.print(",");
    Serial.print(out);
    // Serial.print(",");
    // Serial.print(t);
    Serial.print(",");
    Serial.println(pos);
#endif
  } while (micros() < fin);
  return;
}

/*
*
* MAIN LOOP
*
*/
void loop()
{
  // in = analogRead(INpin);
  //in1 = map(in, 0, 1023, -LIM, LIM);

  // in = analogRead(SENSORpin);
  // in1 = map(in, 0, 1023, -LIM, LIM);

#ifdef DEBUG
  while (Serial.available() > 0)
  {
    ref = Serial.parseFloat();
  }

#endif
  if (ref == -1)
  {
    output(-20);
  }
  if (ref == -2)
  {
    output(20);
  }
  else
  {
    error = ref - pos;
    out = K * error;
    // out = ref;
    output(out);
  }
  // Serial.print("in2: ");
  // Serial.println(in);

  smartDelay(10000);
  // reset current state and error:
}
