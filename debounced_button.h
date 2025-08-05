class DebouncedButton {
private:
  uint8_t pin;
  bool isAnalog;
  uint16_t threshold;
  uint32_t debounceDelay;
  bool lastState;
  bool pressed;
  uint32_t lastChange;

public:
  DebouncedButton(uint8_t pin, bool isAnalog = false, uint16_t threshold = 128, uint32_t debounce = 50)
    : pin(pin), isAnalog(isAnalog), threshold(threshold), debounceDelay(debounce), lastState(false), pressed(false), lastChange(0) {}

  void begin() {
    if (!isAnalog)
      pinMode(pin, INPUT_PULLUP);
  }

  void update() {
    bool currentState;
    if (isAnalog) {
      currentState = analogRead(pin) > threshold;
    } else {
      currentState = !digitalRead(pin);
    }

    if (currentState != lastState && millis() - lastChange > debounceDelay) {
      lastChange = millis();
      if (currentState) pressed = true;
    }

    lastState = currentState;
  }

  bool wasPressed() {
    if (pressed) {
      pressed = false;
      return true;
    }
    return false;
  }

  bool isHeld() const {
    return lastState;
  }
};
