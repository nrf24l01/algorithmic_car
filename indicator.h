#include <FastLED.h>

enum class State {
    Idle,
    Error,
    Writing,
    Working
};

class StatusLEDs {
private:
    CRGB* leds = nullptr;
    uint8_t numLeds = 0;
    State currentState = State::Idle;

public:
    void attach(CRGB* ledArray, uint8_t count) {
        leds = ledArray;
        numLeds = count;
        clear();
        FastLED.show();
    }

    void setState(State newState) {
        if (!leds || numLeds < 3) return;
        if (newState != currentState) {
            currentState = newState;
            applyState();
        }
    }

    State getState() const {
        return currentState;
    }

    void update() {
        FastLED.show();
    }

private:
    void clear() {
        fill_solid(leds, numLeds, CRGB::Black);
    }

    void applyState() {
        clear();

        switch (currentState) {
            case State::Idle:
                leds[1] = CRGB::Yellow;
                break;
            case State::Error:
                leds[1] = CRGB::Red;
                break;
            case State::Writing:
                leds[2] = CRGB::Red;
                break;
            case State::Working:
                leds[0] = CRGB::Green;
                break;
        }

        FastLED.show();
    }
};
