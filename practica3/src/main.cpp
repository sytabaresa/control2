/**
 * GENERAL SYSTEMS CONTROL PROGRAM
 * By: Sebastian Yesid Tabares Amaya
 * Licence: GPLv3
 */

#include <Arduino.h>

// analog input in
#define CURRENTpin A6
#define SENSORpin A0

#define INpin A2

#define ENpin 7
#define PWMpin 9
#define OUTApin 5
#define OUTBpin 4

#define K 1
#define ZERO 511
#define LIM 100

#define CUERPO double

uint16_t out, in, in2 = 0;
CUERPO in1, ref, error = 0;

/**
 * SETUP 
 */
void setup()
{
#ifdef DEBUG
  // Serial begin
  Serial.begin(115200);
  Serial.setTimeout(1500);
#endif

  // analog input config
  pinMode(SENSORpin, INPUT);
  pinMode(CURRENTpin, INPUT);
  pinMode(INpin, INPUT);

  pinMode(ENpin, OUTPUT);
  pinMode(PWMpin, OUTPUT);
  pinMode(OUTApin, OUTPUT);
  pinMode(OUTBpin, OUTPUT);

  digitalWrite(ENpin, HIGH);
}

void output(uint8_t in)
{
  in = in > LIM ? LIM : in;
  in = in < -LIM ? -LIM : in;

  digitalWrite(OUTApin, in > 0);
  digitalWrite(OUTBpin, in < 0);

  in2 = map(abs(in), 0, 100, 0, 255);
  analogWrite(PWMpin, in2);
}



void smartDelay(unsigned long us)
{
  unsigned long fin = micros() + us;
  do
  {

#ifdef DEBUG
  double t = in * 0.02434;
  Serial.print(in);
  Serial.print(",");
  Serial.print(in1);
  Serial.print(",");
  Serial.print(ref);
  Serial.print(",");
  Serial.print(error);
  Serial.print(",");
  Serial.print(out);
  Serial.print(",");
  Serial.println(t);
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

  in = analogRead(SENSORpin);
  in1 = map(in, 0, 1023, -LIM, LIM);

#ifdef DEBUG
  while (Serial.available() > 0)
  {
    ref = Serial.parseInt();
  }
#endif
  error = ref - in1;

  out = map(K * error, -LIM, LIM, 0, 256);
  output(out);

  // Serial.print("in2: ");
  // Serial.println(in);

  smartDelay(10000);
  // reset current state and error:
}