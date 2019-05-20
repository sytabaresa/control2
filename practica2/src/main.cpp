#include <Arduino.h>
#include <LiquidCrystal.h>

// analog input in
#define IN A5

// rising/falling detector pin
#define TRIG A4

// Ts input pin
#define POT A7

// DAC output pins
#define D1 14
#define D2 15
#define D3 16
#define D4 17
#define D5 9
#define D6 10
#define D7 11
#define D8 12

int outPins[] = {D8, D7, D6, D5, D4, D3, D2, D1};

// LCD Display creation
// const int rs = 8, en = 7, d4 = 6, d5 = 5, d6 = 4, d7 = 3;
// LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

// Base Tr
#define TrMax 800
#define Ts TrMax / 10

// global variables
uint8_t pot = 0;
uint16_t in = 0;
uint8_t out = 0;

// Control variables

/**
 *          b0 + b1*z^(-1) + b2*z^(-2) + ... + bn*z^(-n)
 * C(z) = -------------------------------------------------
 *          a0 + a1*z^(-1) + a2*z^(-2) + ... + an*z^(-m)
 * 
 * constA = [a0, a1, a2, ... , an] (length m ==> DEM = m)
 * constB = [b0, b1, b2, ... , bn] (length n ==> NUM = n)
 */

#define CUERPO int32_t //cuerpo para los cálculos (int8_t, int16_t, int32_t, float, double, ...)

#define NUM 2 // n
#define DEM 2 // m

// [e[0], e[1], e[2], ..., e[n]]
CUERPO error[NUM];

// [u[0], u[1], u[2], ..., u[m]]
CUERPO state[DEM];

/*
* controllers:
*/

//unitario
// const CUERPO consB[NUM] = {1};
// const CUERPO consA[DEM] = {1};

// dif atras Ts = Tr / 10
// const CUERPO consA[DEM] = {1468, -1000};
// const CUERPO consB[NUM] = {-6312, 6000};

// dif atras Ts = 7 * Tr
// const CUERPO consA[DEM] = {4360, -1000};
// const CUERPO consB[NUM] = {-8240, 6000};

//dif adelante Ts = Tr / 10
const CUERPO consA[DEM] = {1000, -952};
const CUERPO consB[NUM] = {-6000, 5968};

// dif adelante Ts = 4 * Tr
// const CUERPO consA[DEM] = {1000, 919};
// const CUERPO consB[NUM] = {-6000, 4720};

// trapesoidal Ts = Tr / 10
// const CUERPO consB[NUM] = {-6160, 5840};
// const CUERPO consA[DEM] = {1240, -760};

// trapesoidal Ts = 10 * Tr
// const CUERPO consB[NUM] = {-7600, 4400};
// const CUERPO consA[DEM] = {3400, 1400};

// invarianza al escalón Ts = Tr / 10
// const CUERPO consA[DEM] = {300, -185};
// const CUERPO consB[NUM] = {-1800, 1723};

void output(uint8_t inValue, uint8_t outValue, int16_t inInternal, int16_t outInternal)
{
  // lcd.clear();
  // lcd.setCursor(0, 0);

  pot = analogRead(POT) >> 2;
  pot = map(pot, 0, 255, 1, 100);
  // lcd.print(" Ts/");
  // lcd.print(pot);
  // pot < 10 ? lcd.print(" ") : 0;
  for (int i = 7; i >= 0; i--)
  {
    uint8_t v = (outValue >> i) & 0x01;
    digitalWrite(outPins[i], v);
  }
}

#define TRIGGER 20
void smartDelay(unsigned long us)
{
  // uint16_t out = analogRead(TRIG);
  // uint16_t out1 = out;
  unsigned long fin = micros() + us;
  do
  {
// out = analogRead(TRIG);
// // Serial.print("salida");
// // Serial.println(out);
// if (abs(out - out1) > TRIGGER)
// {
//   // Update registers to new values
//   for (int i = 0; i < DEM; i++)
//     state[i] = 0;

//   for (int i = 0; i < NUM; i++)
//     error[i] = 0;

//   return analogRead(IN);
// }
// out1 = out;
#if DEBUG
    Serial.print(in);
    Serial.print(",");
    Serial.print(error[0]);
    Serial.print(",");
    Serial.print(state[0]);
    Serial.print(",");
    Serial.print(out);
    Serial.print(",");
    Serial.println(pot);

#endif
  } while (micros() < fin);
  return;
}

/**
 * SETUP 
 */
void setup()
{
#ifdef DEBUG
  Serial.begin(115200);
#endif
  // LCD config
  // lcd.begin(16, 2);

  // analog input config
  pinMode(IN, INPUT);

  // sampling time input config
  pinMode(POT, INPUT);

  // DAC outputs config
  for (int i = 0; i < 8; i++)
    pinMode(outPins[i], OUTPUT);

  // initialize control state
  // e[i] = 0
  for (int i = 0; i < NUM; i++)
    error[i] = 0;
  // u[i] = 0
  for (int i = 0; i < DEM; i++)
    state[i] = 0;
}

/*
*
* MAIN LOOP
*
*/
void loop()
{

  // Ts delay and e[k] read in Ts
  in = analogRead(IN);

// scale
#define OFFIN 512

  // input mapping from 12 bits
  error[0] = map(in, 0, 1023, OFFIN, -OFFIN);

  /*
  * u[k] calc:
  */

  //STATE: -an * u[k-n]
  for (int i = 1; i < DEM; i++)
  {
    state[0] -= consA[i] * state[i];
  }

  //ERROR: bn * e[k-n]
  for (int i = 0; i < NUM; i++)
  {
    state[0] += consB[i] * error[i];
  }

  // over a0
  state[0] /= consA[0];

  /*
   * output u[k]
   */

  // saturation of values
  state[0] = state[0] > OFFIN ? OFFIN : state[0];
  state[0] = state[0] < -OFFIN ? -OFFIN : state[0];

  // output mapping to 8 bits
  out = map(state[0], -OFFIN, OFFIN, 0, 255);
  output(in, out, error[0], state[0]);

  // Update registers to new values
  // u[k] = u[k-1]
  for (int i = DEM - 1; i > 0; i--)
    state[i] = state[i - 1];

  // e[k] = e[k-1]
  for (int i = NUM - 1; i > 0; i--)
    error[i] = error[i - 1];

  // reset current state and error:
  // u[k] = 0; e[k] = 0

  smartDelay(Ts / pot * 1000);
  state[0] = 0;
  error[0] = 0;
}