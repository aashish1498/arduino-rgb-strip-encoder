// color swirl! connect an RGB LED to the PWM pins as indicated
// in the #defines
// public domain, enjoy!

#define REDPIN 10
#define GREENPIN 9
#define BLUEPIN 11

enum color {
  RED, BLUE, GREEN, ALL
};
const int LONG_PRESS_TIME = 700;
const int DOUBLE_PRESS_TIME = 300;

const int PinCLK = 2;  // Generating interrupts using CLK signal
const int PinDT = 3;   // Reading DT signal
const int PinSW = 4;   // Reading Push Button switch

bool buttonDown = false;
long buttonDownTime = 0;

bool triggerPending = false;
long triggerPendingTime = 0;

bool buttonTriggered = false;
bool longPress = false;
bool doublePress = false;
bool clickAndHold = false;

int rotaryPosition = 0;
int sensitivity = 3;
int clkState;
int prevClkState;
int r, g, b;

color currentColor = RED;

void setup() {
  pinMode(REDPIN, OUTPUT);
  pinMode(GREENPIN, OUTPUT);
  pinMode(BLUEPIN, OUTPUT);

  pinMode(PinCLK, INPUT);
  pinMode(PinDT, INPUT);
  pinMode(PinSW, INPUT);
  prevClkState = digitalRead(PinCLK);
  digitalWrite(PinSW, HIGH);  // Pull-Up resistor for switch
  Serial.begin(9600);
}

void loop() {
  updateButtonState();
  updateRotaryPosition();
  if (buttonTriggered) {
    Serial.println("Button triggered!");
    goToNextColor();
    buttonTriggered = false;
  }
  if (longPress) {
    Serial.println("Long press!");
    currentColor = ALL;
    longPress = false;
  }
  if (doublePress) {
    Serial.println("Double press!");
    clearColor();
    doublePress = false;
  }
  if (clickAndHold) {
    Serial.println("Clicked and held!");
    setToWhite();
    clickAndHold = false;
  }
  setColor();
  writePins();
}

void writePins() {
  clearSmallValues();
  analogWrite(REDPIN, r);
  analogWrite(GREENPIN, g);
  analogWrite(BLUEPIN, b);
}

void updateButtonState() {
  long currentTime = millis();
  if (!(digitalRead(PinSW)) && !buttonDown) {
    buttonDown = true;
    buttonDownTime = millis();
    // Serial.println("Button down!");
  } else if (digitalRead(PinSW) && buttonDown) {
    buttonDown = false;
    if (currentTime > buttonDownTime + LONG_PRESS_TIME) {
      if (triggerPending) {
        clickAndHold = true;
        triggerPending = false;
      } else {
        longPress = true;
      }
    } else {
      if (triggerPending && currentTime < triggerPendingTime + DOUBLE_PRESS_TIME) {
        triggerPending = false;
        doublePress = true;
      } else if (!triggerPending) {
        triggerPending = true;
        triggerPendingTime = millis();
      }
    }
    // Serial.println("Button up!");
  }
  if (!buttonDown && triggerPending && currentTime > triggerPendingTime + DOUBLE_PRESS_TIME) {
    buttonTriggered = true;
    triggerPending = false;
  }
  // delay(4);
}

void updateRotaryPosition() {
  clkState = digitalRead(PinCLK);
  if (clkState != prevClkState) {
    // If the outputB state is different to the outputA state, that means the encoder is rotating clockwise(?)
    if (digitalRead(PinDT) == clkState) {
      increasePosition();
    } else {
      decreasePosition();
    }
    Serial.print("Position: ");
    Serial.println(rotaryPosition);
  }
  prevClkState = clkState;  // Updates the previous state of the outputA with the current state
  // delay(40);
}

void increasePosition() {
  rotaryPosition = min(rotaryPosition + sensitivity, 255);
}

void decreasePosition() {
  rotaryPosition = max(rotaryPosition - sensitivity, 0);
}

void goToNextColor() {
  if (currentColor == RED) {
    currentColor = GREEN;
    rotaryPosition = g;
  } else if (currentColor == GREEN) {
    currentColor = BLUE;
    rotaryPosition = b;
  } else {
    currentColor = RED;
    rotaryPosition = r;
  }
}

void setColor() {
  switch (currentColor) {
    case RED:
      r = rotaryPosition;
      break;
    case BLUE:
      b = rotaryPosition;
      break;
    case GREEN:
      g = rotaryPosition;
      break;
    case ALL:
      setAllColors(rotaryPosition);
      break;
  }
}

void setToWhite() {
  setAllColors(255);
  rotaryPosition = 255;
  currentColor = RED;
}

void clearColor() {
  setAllColors(0);
  rotaryPosition = 0;
  currentColor = RED;
}

void setAllColors(int value) {
  r = value;
  g = value;
  b = value;
}

void clearSmallValues() {
  if (r <= sensitivity * 2) {
    r = 0;
  }
  if (g <= sensitivity * 2) {
    g = 0;
  }
  if (b <= sensitivity * 2) {
    b = 0;
  }
}