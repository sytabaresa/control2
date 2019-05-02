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
//int out[] = {D1,D2,D3,D4,D5,D6,D7,D8};
int out[] = {D8, D7, D6, D5, D4, D3, D2, D1};

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
#define A0 (1 + 6 * Ts)

#define DEM 2
uint16_t PROGMEM consAoverA0[DEM] = {1, -1 / A0};
uint16_t state[DEM];

#define NUM 2
uint16_t PROGMEM consBoverA0[NUM] = {-(6 + 4 * Ts) / A0, 6 / A0};
uint16_t error[NUM];

void setup()
{
  // LCD config
  lcd.begin(16, 2);

  // analog input config
  pinMode(IN, INPUT);

  // sampling time input config
  pinMode(POT, INPUT);

  // DAC outputs config
  for (int i = 0; i < 8; i++)
    pinMode(out[i], OUTPUT);

  for (int i = 0; i < NUM; i++)
    error[i] = 0;

  for (int i = 0; i < DEM; i++)
    state[i] = 0;
}

void loop()
{
  // e[k] readed
  error[0] = analogRead(IN) >> 2;

  // u[k] calc
  for (int i = 1; i < DEM; i++)
    state[0] += consAoverA0 * state[i];

  for (int i = 0; i < NUM; i++)
    state[0] += consBoverA0 * error[i];

  // lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("in: ");
  in < 10 ? lcd.print(0) : 0;
  lcd.print(in);

  pot = analogRead(POT) >> 2;
  pot = map(pot, 0, 255, 1, 100);
  lcd.print(" Ts/");
  lcd.print(pot);
  pot < 10 ? lcd.print(" ") : 0;

  lcd.setCursor(0, 1);
  lcd.print("out: ");
  for (int i = 7; i >= 0; i--)
  {
    uint8_t v = (state[0] >> i) & 0x01;
    lcd.print(v);

    //show u[k]
    digitalWrite(out[i], v);
  }

  // Update registers to new values
  for (int i = 0; i < DEM - 1; i++)
    state[i + 1] = state[i];

  for (int i = 0; i < NUM - 1; i++)
    error[i + 1] = error[i];

  // Ts delay
  delay(TrMax / pot);
}