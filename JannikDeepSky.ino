/*
Copyright (c) 2021 Jannik Meyer, Lorenz Meyer, Mattis Wacker

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
**/

// Einbinden von Biblitheken zur anzeige auf dem LCD Display
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

#define PULS_OUT_PIN      (9)
#define DIRECTION_OUT_PIN (6)
#define SPEED_POTI_IN_PIN  A1

#define DEFAULT_SPEED     35902
#define RANGE             200
#define UPPER_BOUND       1024 / 2 + (RANGE / 2)
#define LOWER_BOUND       1024 / 2 - (RANGE / 2)
#define SPEED_MULTIPLIER  63

// Initizalizieren des Displays. (I²C Adresse = 0x27, 16 Spalten, 2 Reihen)
// See: https://funduino.de/nr-19-i%C2%B2c-display
LiquidCrystal_I2C lcd(0x27, 16, 2);

void setup()
{
  /* WWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWW */
  /* *********************** Setup des Timers (Start) *********************** */
  // Setze Register zurück
  TCCR1A = 0;
  TCCR1B = 0;

  // Modus Fast PWM-Mode
  TCCR1A |= (1 << WGM11);
  TCCR1B |= (1 << WGM12) | (1 << WGM13);

  // Vorteiler auf 1 setzen -- Vorteiler des Mikrocontroller im Datenblatt suchen
  TCCR1B |= (1 << CS10);

  // Nichtinvertiertes PWM-Signal setzen
  TCCR1A |= (1 << COM1A1);

  // Pulslänge
  OCR1A = 100;

  // Ziel: 32 × Microstepping / Prescale 1 / 80s Versatz
  // Bedingungen:
  // - normaler Modus 200 Steps pro Umdrehung
  // - Microsteps: 32
  // - ---> 6400 Steps pro Umdrehung (steps)
  // - Takt: 16MHz (clk)
  // - Prescaler: 1 (prescale)
  // ----------------------------------------
  // ---> (1/(steps/60[sec/min])) * clk / prescale = 35902
  ICR1 = DEFAULT_SPEED;
  /* ************************ Setup des Timers (Ende) *********************** */
  /* MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM */



  // Setze PULS_OUT_PIN als Ausgang
  pinMode(PULS_OUT_PIN, OUTPUT);

  // Setze DIRECTION_OUT_PIN als Ausgang
  pinMode(DIRECTION_OUT_PIN, OUTPUT);
  // Setze Drehrichtung
  digitalWrite(DIRECTION_OUT_PIN, LOW);
  Serial.begin(9600);
  // kommunikation herstellen
  lcd.init();
  lcd.backlight();
}

int oldValue = 0;
String displayString = "";
String rotationsPerDay = "";
void loop()
{
  int potiValue = analogRead(SPEED_POTI_IN_PIN);

  if (potiValue > LOWER_BOUND && potiValue < UPPER_BOUND && potiValue != oldValue) {
    ICR1 = DEFAULT_SPEED;
    rotationsPerDay = "1x";
    displayString = "STANDARD";
    lcd.noBacklight();
    lcd.clear();
  } else if(potiValue != oldValue) {
    float currentSpeed = DEFAULT_SPEED - (abs(512 - potiValue) * SPEED_MULTIPLIER);
    rotationsPerDay = String(DEFAULT_SPEED / currentSpeed) + "x";
    Serial.println(currentSpeed);
    ICR1 = currentSpeed;
    lcd.backlight();
    lcd.clear();
  }

  if (potiValue < LOWER_BOUND && potiValue != oldValue) {
    // Set Direction To High (Nicht der Default :D)
    digitalWrite(DIRECTION_OUT_PIN, HIGH);
    displayString = "RUECKLAUF";
  }

  if (potiValue > UPPER_BOUND && potiValue != oldValue) {
    // Set Direction To Low (der Default :D)
    digitalWrite(DIRECTION_OUT_PIN, LOW);
    displayString = "VORLAUF";
  }

  // Home the curser
  lcd.setCursor(0, 0);
  lcd.print("Speed: " + rotationsPerDay);
  lcd.setCursor(0, 1);
  lcd.print(displayString);

  oldValue = potiValue;
  delay(50);
}
