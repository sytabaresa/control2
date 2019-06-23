#ifndef __ENCODER_H__
#define __ENCODER_H__

#define DEBOUNCE 30
#define DISTANCE 90

// constructor : sets pins as inputs and turns on pullup resistors
static boolean rotating = false; // debounce management

volatile int16_t position = 0;

int8_t pin_a;

int8_t pin_b;

volatile boolean A_set = false;

volatile boolean B_set = false;

void setEncoder(int8_t PinA, int8_t PinB)
{
  pin_a = PinA;
  pin_b = PinB;

  // set pin a and b to be input
  pinMode(pin_a, INPUT);
  pinMode(pin_b, INPUT);
  // turn on pullup resistors
  digitalWrite(pin_a, HIGH);
  digitalWrite(pin_b, HIGH);
};

// call this from your interrupt function
// Interrupt on A changing state
void doEncoderA()
{
  // debounce
  if (rotating)
    delay(DEBOUNCE); // wait a little until the bouncing is done

  // Test transition, did things really change?
  if (digitalRead(pin_a) != A_set)
  { // debounce once more
    A_set = !A_set;

    // adjust counter + if A leads B
    if (A_set && !B_set)
      // position = position % DISTANCE + 1;
      position++;

    rotating = false; // no more debouncing until loop() hits again
  }
}

// Interrupt on B changing state, same as A above
void doEncoderB()
{
  if (rotating)
    delay(DEBOUNCE);
  if (digitalRead(pin_b) != B_set)
  {
    B_set = !B_set;
    //  adjust counter - 1 if B leads A
    if (B_set && !A_set)
      // position = position == 0 ? DISTANCE : position;
      position--;

    rotating = false;
  }
}

#endif // __ENCODER_H__
