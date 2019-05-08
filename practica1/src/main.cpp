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
uint8_t in = 0;
uint8_t pot = 0;

void output(uint8_t outValue)
{

  // lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("in: ");
  outValue < 10 ? lcd.print(0) : 0;
  lcd.print(outValue);

  pot = analogRead(POT) >> 2;
  pot = map(pot, 0, 255, 1, 100);
  lcd.print(" Ts/");
  lcd.print(pot);
  pot < 10 ? lcd.print(" ") : 0;

  lcd.setCursor(0, 1);
  lcd.print("out: ");
  outValue = map(outValue,0,255,255,0);

  for (int i = 7; i >= 0; i--)
  {
    uint8_t v = (outValue >> i) & 0x01;
    lcd.print(v);

    digitalWrite(outPins[i], v);
  }
}

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
    pinMode(outPins[i], OUTPUT);
}


void loop()
{
  in = analogRead(IN) >> 2;

  output(in);

  // Ts delay
  delay(TrMax / pot);
}