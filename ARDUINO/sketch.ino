#include <Wire.h>
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27, 16, 2);  // address 0x27, 16 columns, 2 rows

const int carGreen = 11;
const int carYellow = 10;
const int carRed = 9;
const int pedGreen = 8;
const int buttonPin = 2;

volatile bool crossingRequested = false;

// Prints text on a row and clears the rest of that row
void printRow(int row, const char* text) {
  lcd.setCursor(0, row);
  lcd.print(text);
  for (int i = strlen(text); i < 16; i++) {
    lcd.print(' ');
  }
}

void showStatus(const char* line1, const char* line2) {
  printRow(0, line1);
  printRow(1, line2);
}

void setup() {
  pinMode(carGreen, OUTPUT);
  pinMode(carYellow, OUTPUT);
  pinMode(carRed, OUTPUT);
  pinMode(pedGreen, OUTPUT);
  pinMode(buttonPin, INPUT_PULLUP);

  lcd.init();
  lcd.backlight();

  // Default state
  digitalWrite(carGreen, HIGH);
  digitalWrite(carYellow, LOW);
  digitalWrite(carRed, LOW);
  digitalWrite(pedGreen, LOW);
  showStatus("Cars: GREEN", "Pedestrian: WAIT");

  attachInterrupt(digitalPinToInterrupt(buttonPin), triggerCrossing, FALLING);
}

void loop() {
  if (crossingRequested) {

    // 1. Vehicle green -> yellow
    digitalWrite(carGreen, LOW);
    digitalWrite(carYellow, HIGH);
    showStatus("Cars: YELLOW", "Pedestrian: WAIT");
    delay(2000);

    // 2. Vehicle red, pedestrian green (with a countdown)
    digitalWrite(carYellow, LOW);
    digitalWrite(carRed, HIGH);
    digitalWrite(pedGreen, HIGH);
    for (int s = 5; s > 0; s--) {
      char line2[17];
      sprintf(line2, "CROSS NOW: %ds", s);
      showStatus("Cars: RED", line2);
      delay(1000);
    }

    // 3. Pedestrian green blinks as a warning
    showStatus("Cars: RED", "Hurry! Ending...");
    for (int i = 0; i < 5; i++) {
      digitalWrite(pedGreen, LOW);
      delay(250);
      digitalWrite(pedGreen, HIGH);
      delay(250);
    }
    digitalWrite(pedGreen, LOW);

    // 4. Back to vehicle green
    digitalWrite(carRed, LOW);
    digitalWrite(carGreen, HIGH);
    showStatus("Cars: GREEN", "Pedestrian: WAIT");

    crossingRequested = false;  // ignore presses made during the sequence
  }
}

void triggerCrossing() {
  static unsigned long lastPress = 0;
  unsigned long now = millis();
  if (now - lastPress > 200) {   // debounce
    crossingRequested = true;
    lastPress = now;
  }
}