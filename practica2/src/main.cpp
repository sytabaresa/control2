#include <Arduino.h>
#include <LiquidCrystal.h>

// analog input in
#define IN A5
#define TRIG A4
// sampling time input pin
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
//int outPins[] = {D1,D2,D3,D4,D5,D6,D7,D8};
int outPins[] = {D8, D7, D6, D5, D4, D3, D2, D1};

// LCD Display creation
const int rs = 8, en = 7, d4 = 6, d5 = 5, d6 = 4, d7 = 3;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

// Base Tr
#define TrMax 780

// global variables
uint8_t pot = 0;

// Control variables

/**
 *          b0 + b1*z^(-1) + b2*z^(-2) + ... + bn*z^(-n)
 * C(z) = -------------------------------------------------
 *          a0 + a1*z^(-1) + a2*z^(-2) + ... + an*z^(-m)
 * 
 * constAoverA0 = [1, a1/a0, a2/a0, ... , an/a0] (length m ==> DEM = m)
 * constBoverB0 = [b0/a0, b1/a0, b2/a0, ... , bn/a0] (length n ==> NUM = n)
 */
#define Ts TrMax / 10
#define CUERPO int32_t

#define DEM 2
// const CUERPO consA[DEM] = {1};
const CUERPO consA[DEM] = {-1468, 1000}; // dif adelante
// const CUERPO consA[DEM] = {1000, -520}; // dif atras
// const CUERPO consA[DEM] = {1240, -760}; // trapesoidal
// const CUERPO consA[DEM] = {300, -185}; // invarianza al escalóñ

CUERPO state[DEM];

#define NUM 2
// const CUERPO consB[NUM] = {1};
const CUERPO consB[NUM] = {6312, -6000}; //dif adelante
// const CUERPO consB[NUM] = {-6000, 5680}; //dif atras
// const CUERPO consB[NUM] = {-6312, 6000}; //dif adelante
// const CUERPO consB[NUM] = {-6160, 5840}; //trapesoidal
// const CUERPO consB[NUM] = {-1800, 3723}; // invarianza al escalóñ


CUERPO error[NUM];

void output(uint8_t inValue, uint8_t outValue, int16_t inInternal, int16_t outInternal)
{

  lcd.clear();
  lcd.setCursor(0, 0);
  // lcd.print("in: ");
  // inValue < 10 ? lcd.print(000) : 0;
  // inValue < 100 ? lcd.print(00) : 0;
  // inValue < 1000 ? lcd.print(0) : 0;
  // lcd.print(inValue);
  // lcd.print(" ");
  // lcd.print(inInternal);

  pot = analogRead(POT) >> 2;
  pot = map(pot, 0, 255, 1, 100);
  lcd.print(" Ts/");
  lcd.print(pot);
  pot < 10 ? lcd.print(" ") : 0;

  // lcd.setCursor(0, 1);
  // lcd.print("out: ");
  // lcd.print(outValue);
  // lcd.print(" ");
  // lcd.print(outInternal);
  // outValue = map(outValue, 0, 255, 255, 0);

  for (int i = 7; i >= 0; i--)
  {
    uint8_t v = (outValue >> i) & 0x01;
    digitalWrite(outPins[i], v);
  }
}

#define TRIGGER 20
uint16_t smartDelay(unsigned long ms)
{
  // uint16_t out = analogRead(TRIG);
  // uint16_t out1 = out;
  unsigned long fin = millis() + ms;
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
  } while (millis() < fin);
  return analogRead(IN);
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
  lcd.begin(16, 2);

  // analog input config
  pinMode(IN, INPUT);

  // sampling time input config
  pinMode(POT, INPUT);

  // DAC outputs config
  for (int i = 0; i < 8; i++)
    pinMode(outPins[i], OUTPUT);

  for (int i = 0; i < NUM; i++)
    error[i] = 0;

  for (int i = 0; i < DEM; i++)
    state[i] = 0;
}

void loop()
{

#ifdef DEBUG
  Serial.print("state: [");
  for (int i = 0; i < DEM; i++)
  {
    Serial.print(state[i]);
    Serial.print(", ");
  }
  Serial.println("]");
  Serial.print("error: [");
  for (int i = 0; i < NUM; i++)
  {
    Serial.print(error[i]);
    Serial.print(", ");
  }
  Serial.println("]");

#endif

  // e[k] readed in Ts
  uint16_t in = smartDelay(TrMax / pot);
  // uint16_t in = analogRead(IN);

#define OFFIN 200
#define LIM 10000000
  error[0] = map(in, 0, 1023, OFFIN, -OFFIN);
#ifdef DEBUG
  Serial.print("in: ");
  Serial.print(in);
  Serial.print(", e[k]: ");
  Serial.println(error[0]);
#endif
  // u[k] calc

  //STATE
  for (int i = 1; i < DEM; i++)
  {
    state[0] -= consA[i] * state[i];
#ifdef DEBUG
    Serial.print("state");
    Serial.print(i);
    Serial.print(": ");
    Serial.println(consA[i] * state[i]);
#endif
  }

  //ERROR
  for (int i = 0; i < NUM; i++)
  {
    state[0] += consB[i] * error[i];
#ifdef DEBUG
    Serial.print("error");
    Serial.print(i);
    Serial.print(": ");
    Serial.println(consB[i] * error[i]);
#endif
  }

#ifdef DEBUG
  Serial.print("pstate0: ");
  Serial.println(state[0]);
#endif
  // state[0] = state[0] > LIM ? LIM : state[0];
  // state[0] = state[0] < -LIM ? -LIM : state[0];
  state[0] /= consA[0];
#ifdef DEBUG
  Serial.print("state0: ");
  Serial.println(state[0]);
#endif

  // output u[k]
  //#define OFFOUT 1000
  state[0] = state[0] > OFFIN ? OFFIN : state[0];
  state[0] = state[0] < -OFFIN ? -OFFIN : state[0];
  uint8_t out = map(state[0], -OFFIN, OFFIN, 0, 255);
#ifdef DEBUG
  Serial.print("u[k]: ");
  Serial.print(state[0]);
  Serial.print(", out: ");
  Serial.println(out);
#endif
  output(in, out, error[0], state[0]);

  // Update registers to new values
  for (int i = DEM - 1; i > 0; i--)
    state[i] = state[i - 1];

  for (int i = NUM - 1; i > 0; i--)
    error[i] = error[i - 1];

#ifdef DEBUG
  Serial.print("out: ");
  Serial.println(out);
  Serial.println();
  Serial.println();

#endif
  state[0] = 0;
  error[0] = 0;

  // Ts delay
  // delay(TrMax / pot);
  // delay(1000);
}