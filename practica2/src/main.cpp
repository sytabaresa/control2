/**
 * GENERAL SYSTEMS CONTROL PROGRAM
 * By: Sebastian Yesid Tabares Amaya
 * Licence: GPLv3
 */

#include <Arduino.h>

// analog input in
#define IN A5

// DAC output pins
#define D1 14
#define D2 15
#define D3 16
#define D4 17
#define D5 9
#define D6 10
#define D7 11
#define D8 12

// MCU pin mapping for DAC
int outPins[] = {D8, D7, D6, D5, D4, D3, D2, D1};

/*
 * Global variables
 */
uint16_t in = 0;
uint8_t out = 0;

/**
 * Control variables:
 * 
 *          b0 + b1*z^(-1) + b2*z^(-2) + ... + bn*z^(-n)
 * C(z) = -------------------------------------------------
 *          a0 + a1*z^(-1) + a2*z^(-2) + ... + an*z^(-m)
 * 
 * constA = [a0, a1, a2, ... , an] (length m ==> DEN = m)
 * constB = [b0, b1, b2, ... , bn] (length n ==> NUM = n)
 */

#define FIELD double //FIELD for the calcs (int8_t, int16_t, int32_t, float, double, ...)

/*
 * Numerator and Denominator order, very important to calculate the size of error and state arrays
 */
#define NUM 2 // NUM -> n
#define DEN 2 // DEN -> m

// [e[k], e[k-1], e[k-2], ..., e[k-n]]
FIELD error[NUM];

// [u[k], u[k-1], u[k-2], ..., u[k-m]]
FIELD state[DEN];

#define TrMax 800000  // Base Tr
#define Ts TrMax / 10 //  Tr/12 < Ts < Tr/8

/*
 * IMPORTANT: comment and uncomment acording to the controllers
 */

// #define T Ts / 10
#define T Ts
// #define n 5 // n is the multiplicator for the controller with this coeff.
// #define T n * Ts

/**
* Controllers:
* comment and uncomment according to controller in testing, see also T and NUM/DEN definition above
*/

// ZOH (NUM = 1 and DEN = 1)
// const FIELD consB[NUM] = {1};
// const FIELD consA[DEN] = {1};

// Backward difference T = Ts / 10
const FIELD consA[DEN] = {1048, -1000};
const FIELD consB[NUM] = {-6032, 6000};

// Backward difference T = Ts
// const FIELD consA[DEN] = {1468, -1000};
// const FIELD consB[NUM] = {-6312, 6000};

// Backward difference T = 7 * Ts
// const FIELD consA[DEN] = {4360, -1000};
// const FIELD consB[NUM] = {-8240, 6000};

// Forward difference T = Ts / 10
//  const FIELD consA[DEN] = {1000, -952};
//  const FIELD consB[NUM] = {-6000, 5968};

// Forward difference T = Ts
//  const FIELD consA[DEN] = {1000, -520};
//  const FIELD consB[NUM] = {-6000, 5680};

// Forward difference T = 4 * Ts
// const FIELD consA[DEN] = {1000, 919};
// const FIELD consB[NUM] = {-6000, 4720};

// Forward difference T = 5 * Ts (doesn't work)
// const FIELD consA[DEN] = {1000, 1400};
// const FIELD consB[NUM] = {-6000, 4400};

// Tustin’s approximation T = Ts / 10
// const FIELD consB[NUM] = {-6016, 5984};
// const FIELD consA[DEN] = {1024, -976};

// Tustin’s approximation T = 10 * Ts
// const FIELD consA[DEN] = {3400, 1400};
// const FIELD consB[NUM] = {-7600, 4400};

// Tustin’s approximation T = Ts
// const FIELD consB[NUM] = {-6160, 5840};
// const FIELD consA[DEN] = {1240, -760};

// Step invariance T = Ts / 10
//  const FIELD consA[DEN] = {300, -285};
//  const FIELD consB[NUM] = {-1800, 1790};

// Step invariance  T = Ts
// const FIELD consA[DEN] = {300, -185};
// const FIELD consB[NUM] = {-1800, 1723};

// Step invariance  T = 5 * Ts
// const FIELD consA[DEN] = {300, -27.21};
// const FIELD consB[NUM] = {-1800, 1618};

// Step invariance  T = 4 * Ts
// const FIELD consA[DEN] = {300, -44};
// const FIELD consB[NUM] = {-1800, 1629};

void smartDelay(unsigned long us)
{
  unsigned long fin = micros() + us;
  do
  {

#if DEBUG
    Serial.print(in);
    Serial.print(",");
    Serial.print(error[0]);
    Serial.print(",");
    Serial.print(state[0]);
    Serial.print(",");
    Serial.print(out);

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
  // Serial begin
  Serial.begin(115200);
#endif

  // analog input config
  pinMode(IN, INPUT);

  // DAC outputs config
  for (int i = 0; i < 8; i++)
    pinMode(outPins[i], OUTPUT);

  // initialize control state
  // e[k-i] = 0
  for (int i = 0; i < NUM; i++)
    error[i] = 0;
  // u[k-i] = 0
  for (int i = 0; i < DEN; i++)
    state[i] = 0;
}

/*
*
* MAIN LOOP
*
*/
void loop()
{
  // reset current state and error:
  // e[k] = 0 and u[k] = 0
  state[0] = 0;
  error[0] = 0;

  // Ts delay and e[k] read in Ts
  in = analogRead(IN);

// scale
#define OFFIN 512
#define FIX 20
  // input mapping from 12 bits
  error[0] = map(in, 0, 1023, OFFIN + FIX, -OFFIN + FIX);

  /*
  * u[k] calc:
  */
  //STATE: -an * u[k-n]
  for (int i = 1; i < DEN; i++)
    state[0] -= consA[i] * state[i];

  //ERROR: bn * e[k-n]
  for (int i = 0; i < NUM; i++)
    state[0] += consB[i] * error[i];

  // over a0
  state[0] /= consA[0];

  /*
   * output u[k]
   */
  // saturation of values
  state[0] = state[0] > OFFIN ? OFFIN : state[0];
  state[0] = state[0] < -OFFIN ? -OFFIN : state[0];

  // output mapping to 8 bits DAC
  out = map(state[0], -OFFIN, OFFIN, 0, 255);
  for (int i = 7; i >= 0; i--)
  {
    uint8_t v = (out >> i) & 0x01;
    digitalWrite(outPins[i], v);
  }

  /**
  * Update registers to new values
  */
  // u[k] = u[k-1]
  for (int i = DEN - 1; i > 0; i--)
    state[i] = state[i - 1];

  // e[k] = e[k-1]
  for (int i = NUM - 1; i > 0; i--)
    error[i] = error[i - 1];

  // T delay for time and value hold
  smartDelay(T);
}