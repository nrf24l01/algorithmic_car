#include <LiquidCrystal_I2C.h>
#include "debounced_button.h"

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
byte path[160];
int custep = 0;
bool pathing = false;

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
  B00100, B01110, B10101, B00100, B00100, B00100, B00100, B00000,
};
byte CH_DOWN_ARROW[] = {
  B00000, B00100, B00100, B00100, B00100, B10101, B01110, B00100,
};
byte CH_RIGHT_ARROW[] = {
  B00000, B00100, B00010, B11111, B00010, B00100, B00000, B00000,
};
byte CH_LEFT_ARROW[] = {
  B00000, B00000, B00100, B01000, B11111, B01000, B00100, B00000,
};
byte CH_DOT[] = {
  B00000, B00000, B00000, B01110, B01110, B01110, B00000, B00000,
};
byte CH_CLOSE_LOOP[] = {
  B00000, B01000, B00100, B00010, B00010, B00100, B01000, B00000,
};
byte CH_OPEN_LOOP[] = {
  B00000, B00010, B00100, B01000, B01000, B00100, B00010, B00000,
};

void setup() {
  Serial.begin(9600);

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

  // Обработка записи
  pathing = digitalRead(BTN_TOGGLE_RECORD);

  if (pathing) {
    if (btnForward.wasPressed()) {
      path[custep++] = UP;
      path[custep++] = 1;
      Serial.println("UP");
    }

    if (btnBackward.wasPressed()) {
      path[custep++] = DOWN;
      path[custep++] = 1;
      Serial.println("DOWN");
    }

    if (btnLeft.wasPressed()) {
      path[custep++] = LEFT;
      path[custep++] = 1;
      Serial.println("LEFT");
    }

    if (btnRight.wasPressed()) {
      path[custep++] = RIGHT;
      path[custep++] = 1;
      Serial.println("RIGHT");
    }

    if (btnOpenLoop.wasPressed()) {
      path[custep++] = FORS;
      path[custep++] = 0;
      Serial.println("OPEN (");
    }

    if (btnCloseLoop.wasPressed()) {
      path[custep++] = FORE;
      path[custep++] = 2;
      Serial.println("CLOSE )");
    }

    if (btnPlus.wasPressed() && custep >= 1) {
      path[custep - 1]++;
      Serial.println("INCREMENT");
      lcd.clear();
    }

    if (btnMinus.wasPressed() && custep >= 1) {
      path[custep - 1]--;
      Serial.println("DECREMENT");
      lcd.clear();
    }

    if (btnDelete.wasPressed() && custep >= 2) {
      custep -= 2;
      path[custep] = PASS;
      path[custep + 1] = PASS;
      Serial.println("DELETE");
      lcd.clear();
    }
  }

  // === Отображение ===
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
      lcd.setCursor(x++, y);
      lcd.print(param);
    }
  }

  delay(10);
}
