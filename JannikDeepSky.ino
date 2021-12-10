/*
 * Copyright (c) 2021 Jannik Meyer, Lorenz Meyer, Mattis Wacker
 */

// Include necessary libraries to communicate with the display
#include <LiquidCrystal_I2C.h>

// Pins
#define PULS_OUT_PIN      9
#define DIRECTION_OUT_PIN 6
#define SPEED_POTI_IN_PIN A1

// Constants
#define DEFAULT_SPEED     35902
#define RANGE             200
#define ANALOG_RESOLUTION 1024
#define UPPER_BOUND       ANALOG_RESOLUTION / 2 + (RANGE / 2)
#define LOWER_BOUND       ANALOG_RESOLUTION / 2 - (RANGE / 2)
#define SPEED_MULTIPLIER  63

// Initialize the display. (I²C Adresse = 0x27, 16 Spalten, 2 Reihen)
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

  // Set PULS_OUT_PIN to output
  pinMode(PULS_OUT_PIN, OUTPUT);

  // Set DIRECTION_OUT_PIN to output
  pinMode(DIRECTION_OUT_PIN, OUTPUT);
  
  // Setze Drehrichtung
  digitalWrite(DIRECTION_OUT_PIN, LOW);
  
  // Initialize the communication to the lcd display
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
    // When the poti is in the middle section set the speed to the needed default speed to achieve the desired speed of 1 rotation per minute
    ICR1 = DEFAULT_SPEED;
    rotationsPerDay = "1x";
    displayString = "Standard";
    
    // Set direction to low (Default Direction)
    digitalWrite(DIRECTION_OUT_PIN, LOW);

    // Turn the backlight of the display of, to not interfere with the image
    lcd.noBacklight();
  } else if(potiValue != oldValue) {
    // When the poti is outside the middle section use the absolute distance from the center position to reduce the time until the next tick to the stepper motor is send, increasing its speed.
    float currentSpeed = DEFAULT_SPEED - (abs((ANALOG_RESOLUTION / 2) - potiValue) * SPEED_MULTIPLIER);

    // Calculate the rotations per day by dividing the current speed by the current speed and set the value for the timer to count to, to this value.
    rotationsPerDay = String(DEFAULT_SPEED / currentSpeed) + "x";
    ICR1 = currentSpeed;

    // Turn the backlight on when adjusting the position of the camera
    lcd.backlight();
    lcd.clear();
  }

  if (potiValue < LOWER_BOUND && potiValue != oldValue) {
    // Set direction to high, Reversing the direction of the stepper motor
    digitalWrite(DIRECTION_OUT_PIN, HIGH);
    displayString = "Ruecklauf";
  }

  if (potiValue > UPPER_BOUND && potiValue != oldValue) {
    // Set direction to low (Default Direction)
    digitalWrite(DIRECTION_OUT_PIN, LOW);
    displayString = "Vorlauf";
  }

  // Print the information to the display
  lcd.setCursor(0, 0);
  lcd.print("Speed: " + rotationsPerDay + "       ");
  lcd.setCursor(0, 1);
  lcd.print(displayString + "       ");

  // Store the old value of the potentiometer, to check for changes
  oldValue = potiValue;
  delay(10);
}
