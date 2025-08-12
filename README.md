# Algorithmic car
- Based on arduino nano
- 2 A4988 drivers for stepper mottors
- 9 configure buttons
- 1 button for start
- switch to select edit/run mode
- ws2812b led light to show status

| Назначение          | Пин  |
|---------------------|------|
| LED                 | 2    |
| Кнопка вперёд       | 4    |
| Кнопка назад        | 5    |
| Кнопка поворот влево| 6    |
| Кнопка поворот вправо| 7   |
| Кнопка open loop    | 8    |
| Кнопка close loop   | 9    |
| Повтор +            | 10   |
| Повтор −            | 11   |
| Старт выполнения    | 12   |
| Кнопка записи       | 3    |
| Удалить последнюю   | A7   |
| Левый шаговик DIR   | A1   |
| Левый шаговик STEP  | A0   |
| Правый шаговик DIR  | A3   |
| Правый шаговик STEP | A2   |
| Левый/правый шаговик EN | 13 |
| LCD (I2C)           | SDA/SCL (адрес 0x27) |
