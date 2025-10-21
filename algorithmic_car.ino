#include <LiquidCrystal_I2C.h>
#include <GyverStepper.h>
#include <FastLED.h>

#define LED_PIN 2
#define NUM_LEDS 3

#define PATH_SIZE 160
#define PATH_COMPILED_SIZE 256

#define MOVS 1428
#define ROTS 930

GStepper<STEPPER2WIRE> left_stepper(800, A1, A0, 13);
GStepper<STEPPER2WIRE> right_stepper(800, A3, A2, 13);

// === Пины кнопок ===
#define BTN_FORWARD        4
#define BTN_BACKWARD       5
#define BTN_TURN_LEFT      6
#define BTN_TURN_RIGHT     7
#define BTN_OPEN_LOOP      8
#define BTN_CLOSE_LOOP     9
#define BTN_REPEAT_PLUS   10
#define BTN_REPEAT_MINUS  11
#define BTN_START_EXEC    12
#define BTN_DELETE_LAST   A7
#define BTN_TOGGLE_RECORD  3

// === Символьные значения команд ===
#define PASS  0
#define UP    1
#define LEFT  2
#define RIGHT 3
#define DOWN  4
#define FORS  5
#define FORE  6

// === Экран ===
#define SCREEN_SIZE_X 20
#define SCREEN_SIZE_Y 4
#define I2C_ADDR_ASS 0x27

LiquidCrystal_I2C lcd(I2C_ADDR_ASS, SCREEN_SIZE_X, SCREEN_SIZE_Y);

// === Пути ===
byte path[PATH_SIZE];
int custep = 0;
bool pathing = false;

bool moving = false;
bool need_compile = true;
bool compile_success = false;

#include "debounced_button.h"
#include "movement.h"
#include "indicator.h"

CRGB leds[NUM_LEDS];
StatusLEDs status;

// === Объекты кнопок ===
DebouncedButton btnForward(BTN_FORWARD);
DebouncedButton btnBackward(BTN_BACKWARD);
DebouncedButton btnLeft(BTN_TURN_LEFT);
DebouncedButton btnRight(BTN_TURN_RIGHT);
DebouncedButton btnOpenLoop(BTN_OPEN_LOOP);
DebouncedButton btnCloseLoop(BTN_CLOSE_LOOP);
DebouncedButton btnPlus(BTN_REPEAT_PLUS);
DebouncedButton btnMinus(BTN_REPEAT_MINUS);
DebouncedButton btnStartExec(BTN_START_EXEC);
DebouncedButton btnDelete(BTN_DELETE_LAST, true);  // analog

// === Символы ===
byte CH_UP_ARROW[] = {
  B00100, B01110, B11111, B00100, B00100, B00100, B00100, B00100,
};
byte CH_DOWN_ARROW[] = {
  B00100, B00100, B00100, B00100, B00100, B11111, B01110, B00100,
};
byte CH_RIGHT_ARROW[] = {
  B00000, B00100, B00010, B11111, B00010, B00100, B00000, B00000,
};
byte CH_LEFT_ARROW[] = {
  B00000, B00100, B01000, B11111, B01000, B00100, B00000, B00000,
};
byte CH_DOT[] = {
  B00000, B00000, B01110, B01110, B01110, B00000, B00000, B00000,
};
byte CH_CLOSE_LOOP[] = {
  B01000, B00100, B00100, B00010, B00010, B00100, B00100, B01000,
};
byte CH_OPEN_LOOP[] = {
  B00010, B00100, B00100, B01000, B01000, B00100, B00100, B00010,
};

void setup() {
  Serial.begin(9600);
  
  pinMode(BTN_TOGGLE_RECORD, INPUT_PULLUP);
  
  // Steppers setup
  setup_steppers();

  // Инициализация кнопок
  btnForward.begin();
  btnBackward.begin();
  btnLeft.begin();
  btnRight.begin();
  btnOpenLoop.begin();
  btnCloseLoop.begin();
  btnPlus.begin();
  btnMinus.begin();
  btnStartExec.begin();
  btnDelete.begin();

  // LCD init
  lcd.init();
  lcd.backlight();

  lcd.createChar(0, CH_UP_ARROW);
  lcd.createChar(1, CH_DOWN_ARROW);
  lcd.createChar(2, CH_RIGHT_ARROW);
  lcd.createChar(3, CH_LEFT_ARROW);
  lcd.createChar(4, CH_DOT);
  lcd.createChar(5, CH_CLOSE_LOOP);
  lcd.createChar(6, CH_OPEN_LOOP);

  for (int i = 0; i < 160; i++) path[i] = PASS;

  FastLED.addLeds<WS2812B, LED_PIN, GRB>(leds, NUM_LEDS);
  status.attach(leds, NUM_LEDS);
  status.setState(State::Error);
  status.setState(State::Writing);
  status.setState(State::Working);
  status.setState(State::Idle);
}

int numDigits(uint8_t num) {
  if (num == 0) return 1;
  int digits = 0;
  while (num > 0) {
    num /= 10;
    digits++;
  }
  return digits;
}

void loop() {
  // Обновление всех кнопок
  btnForward.update();
  btnBackward.update();
  btnLeft.update();
  btnRight.update();
  btnOpenLoop.update();
  btnCloseLoop.update();
  btnPlus.update();
  btnMinus.update();
  btnStartExec.update();
  btnDelete.update();
  bool redraw = false;

  // Обработка записи
  pathing = digitalRead(BTN_TOGGLE_RECORD);

  if (pathing && !moving) {
    status.setState(State::Writing);
    if (btnForward.wasPressed()) {
      path[custep++] = UP;
      path[custep++] = 1;
      Serial.println("UP");
      redraw = true;
    }

    if (btnBackward.wasPressed()) {
      path[custep++] = DOWN;
      path[custep++] = 1;
      Serial.println("DOWN");
      redraw = true;
    }

    if (btnLeft.wasPressed()) {
      path[custep++] = LEFT;
      path[custep++] = 1;
      Serial.println("LEFT");
      redraw = true;
    }

    if (btnRight.wasPressed()) {
      path[custep++] = RIGHT;
      path[custep++] = 1;
      Serial.println("RIGHT");
      redraw = true;
    }

    if (btnOpenLoop.wasPressed()) {
      path[custep++] = FORS;
      path[custep++] = 0;
      Serial.println("OPEN (");
      redraw = true;
    }

    if (btnCloseLoop.wasPressed()) {
      path[custep++] = FORE;
      path[custep++] = 2;
      Serial.println("CLOSE )");
      redraw = true;
    }

    if (btnPlus.wasPressed() && custep >= 1) {
      if (path[custep - 1] < 255) {
        path[custep - 1]++;
        Serial.println("INCREMENT");
        lcd.clear();
        redraw = true;
      }
    }

    if (btnMinus.wasPressed() && custep >= 1) {
      if ((path[custep-2] == FORE && path[custep - 1]>2) || (path[custep-2] != FORE && path[custep - 1]>1)){
        path[custep - 1]--;
        Serial.println("DECREMENT");
        redraw = true;
        lcd.clear();
      }
    }

    if (btnDelete.wasPressed() && custep >= 2) {
      custep -= 2;
      path[custep] = PASS;
      path[custep + 1] = PASS;
      Serial.println("DELETE");
      redraw = true;
      lcd.clear();
    }
    need_compile = true;
  } else {
    if (need_compile) {
      compile_success = compile_path();
      need_compile = false;
      moving = false;
      if (compile_success) {
        status.setState(State::Idle);
      } else {
        status.setState(State::Error);
      }
    }
    if (btnStartExec.wasPressed() && compile_success) {
      path_point = 0;
      moving = true;
    }
    if (moving) {
      status.setState(State::Working);
      bool r = move();
      if (r) {
        moving = false;
        status.setState(State::Idle);
      }
    }
  }

  // === Отображение ===
  if (redraw) {
    lcd.setCursor(0, 0);
    int x = 0, y = 0;
    for (int i = 0; i < custep; i += 2) {
      if (x >= SCREEN_SIZE_X) {
        x = 0;
        y++;
        if (y >= SCREEN_SIZE_Y) break;
      }

      byte cmd = path[i];
      byte param = path[i + 1];

      lcd.setCursor(x++, y);
      switch (cmd) {
        case UP:    lcd.write((uint8_t)0); break;
        case DOWN:  lcd.write((uint8_t)1); break;
        case RIGHT: lcd.write((uint8_t)2); break;
        case LEFT:  lcd.write((uint8_t)3); break;
        case FORS:  lcd.write((uint8_t)6); break;
        case FORE:
          lcd.write((uint8_t)5);
          if (x >= SCREEN_SIZE_X) { x = 0; y++; }
          lcd.setCursor(x++, y);
          lcd.write((uint8_t)4); // точка
          break;
        default: break;
      }

      if (param > 1) {
        if (x >= SCREEN_SIZE_X) { x = 0; y++; }
        lcd.setCursor(x, y);
        lcd.print(param);
        x += numDigits(param);  // корректно продвигаем курсор
      }
    }
  }
}
