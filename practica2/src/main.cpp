#include <Arduino.h>
#include <LiquidCrystal.h>

// analog input in
#define IN A5

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
#define A0 (1.0 + 6 * Ts)

#define DEM 2
const double consAoverA0[DEM] = {1.0, -1.0 / A0};
double state[DEM];

#define NUM 2
const double consBoverA0[NUM] = {-(6.0 + 4 * Ts) / A0, 6.0 / A0};
//const double consBoverA0[NUM] = {(-0.6780383795309168349), (0.01279317697228144965)};
double error[NUM];

void output(uint8_t inValue, uint8_t outValue, int16_t inInternal, int16_t outInternal)
{

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("in: ");
  inValue < 10 ? lcd.print(000) : 0;
  inValue < 100 ? lcd.print(00) : 0;
  inValue < 1000 ? lcd.print(0) : 0;
  lcd.print(inValue);
  lcd.print(" ");
  lcd.print(inInternal);

  pot = analogRead(POT) >> 2;
  pot = map(pot, 0, 255, 1, 100);
  lcd.print(" Ts/");
  lcd.print(pot);
  pot < 10 ? lcd.print(" ") : 0;

  lcd.setCursor(0, 1);
  lcd.print("out: ");
  lcd.print(outValue);
  lcd.print(" ");
  lcd.print(outInternal);
  outValue = map(outValue, 0, 255, 255, 0);

  for (int i = 7; i >= 0; i--)
  {
    uint8_t v = (outValue >> i) & 0x01;
    digitalWrite(outPins[i], v);
  }
}

double doubleMap(double x, double in_min, double in_max, double out_min, double out_max)
{
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

/**
 * SETUP 
 */
void setup()
{
#ifdef DEBUG
  Serial.begin(9600);
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
  // e[k] readed
  uint16_t in = analogRead(IN);

#define OFFIN 1000
  error[0] = doubleMap(in, 0, 1023, -OFFIN, OFFIN);
#ifdef DEBUG
  Serial.print("in1: ");
  Serial.print(in);
  Serial.print(", in2: ");
  Serial.println(error[0]);
#endif
  // u[k] calc

  //STATE
  for (int i = 1; i < DEM; i++)
  {
    state[0] += consAoverA0[i] * state[i];
#ifdef DEBUG
    Serial.print("st");
    Serial.print(i);
    Serial.print(": ");
    Serial.println(consAoverA0[i] * state[i]);
#endif
  }

  //ERROR
  for (int i = 0; i < NUM; i++)
  {
    state[0] += consBoverA0[i] * error[i];
#ifdef DEBUG
    Serial.print("err");
    Serial.print(i);
    Serial.print(": ");
    Serial.println(consBoverA0[i] * error[i]);
#endif
  }

// output u[k]
//#define OFFOUT 1000
  uint8_t out = map(state[0], -OFFIN, OFFIN, 0, 255);
#ifdef DEBUG
  Serial.print("out1: ");
  Serial.print(state[0]);
  Serial.print(", in2: ");
  Serial.println(out);
#endif
  output(in, out, error[0], state[0]);

  // Update registers to new values
  for (int i = 0; i < DEM - 1; i++)
    state[i + 1] = state[i];

  for (int i = 0; i < NUM - 1; i++)
    error[i + 1] = error[i];

#ifdef DEBUG
  Serial.print("state: ");
  for (int i = 0; i < DEM; i++)
  {
    Serial.print(state[i]);
    Serial.print(", ");
  }
  Serial.println();
  Serial.print("error: ");
  for (int i = 0; i < NUM; i++)
  {
    Serial.print(error[i]);
    Serial.print(", ");
  }
  Serial.println();
  Serial.println();

#endif
  // Ts delay
delay(TrMax / pot);
 // delay(1000);
}