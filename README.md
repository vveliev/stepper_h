# stepper_h
Non-blocking stepper motor control library for ChipKIT (Arduino-compatible pic32-based dev board)

Hello,

Here is my library to control multiple stepper motors connected to ChipKIT board via step-dir driver interface
https://github.com/1i7/stepper_h

The major benefit in comparison to Arduino Stepper.h lib https://www.arduino.cc/en/Reference/StepperStep is that my stepper_h calls are non-blocking: you can run simultaneously multiple motors and receive commands via Wify, Serial port and do anything else in the main loop at the same time.

It uses PIC32 timer interrupts to generate step signals in background (basic code to init timer taken from ChipKIT Servo.h library port) and ChipKIT/Arduino API to deal with pins, so it would work only on ChipKIT boards with Arduino-compatible firmware (will not work on classic Arduino or pure PIC32 chip).

to install, just make git clone https://github.com/1i7/stepper_h to ~/Arduino/libraries (for the new ChipKIT IDE)

```bash
cd ~/Arduino/libraries
git clone https://github.com/1i7/stepper_h.git
```

basic example should appear in Arduino examples menu: File/Examples/stepper_h/stepper_h

This one runs 3 stepper motors with different speed at the same time. Motors start to run after calling stepper_start_cycle(). Note, that loop can contain any code (or have no code at all) - the motors would work in background.
```c++
#include "stepper.h"

// Stepper motors
static stepper sm_x, sm_y, sm_z;

static void prepare_line1() {
    // prepare_steps(stepper *smotor,
    //     long step_count, unsigned long step_delay,
    //     calibrate_mode_t calibrate_mode=NONE);

    // make 20000 steps with 1000 microseconds delay
    // X.pos would go from 0 to
    // 7500*20000=150000000 nanometers = 150000 micrometers = 150 millimeters
    // during 1000*20000=20000000microseconds=20seconds
    prepare_steps(&sm_x, 20000, 1000);
    // make 10000 steps with 2000 microseconds delay
    // Y.pos would go from 0 to
    // 7500*10000=75000000 nanometers = 75000 micrometers = 75 millimeters
    // during 2000*10000=20000000microseconds=20seconds
    prepare_steps(&sm_y, 10000, 2000);
    // make 1000 steps with 20000 microseconds delay
    // Z.pos would go from 0 to
    // 7500*1000=7500000 nanometers = 7500 micrometers  = 7.5 millimeters
    // during 20000*1000=20000000microseconds=20seconds
    prepare_steps(&sm_z, 1000, 20000);
}

void setup() {
    Serial.begin(9600);
    Serial.println("Starting stepper_h test...");
    
    // connected stepper motors
    // init_stepper(stepper* smotor, char name,
    //     int pin_step, int pin_dir, int pin_en,
    //     bool invert_dir, int step_delay,
    //     int distance_per_step)
    // init_stepper_ends(stepper* smotor,
    //     end_strategy min_end_strategy, end_strategy max_end_strategy,
    //     long long min_pos, long long max_pos);
    
    // X
    init_stepper(&sm_x, 'x', 8, 9, 10, false, 1000, 7500);
    init_stepper_ends(&sm_x, NO_PIN, NO_PIN, CONST, CONST, 0, 300000000);
    // Y
    init_stepper(&sm_y, 'y', 5, 6, 7, true, 1000, 7500);
    init_stepper_ends(&sm_y, NO_PIN, NO_PIN, CONST, CONST, 0, 216000000);
    // Z
    init_stepper(&sm_z, 'z', 2, 3, 4, true, 1000, 7500);
    init_stepper_ends(&sm_z, NO_PIN, NO_PIN, CONST, CONST, 0, 100000000);
    
    // configure motors before starting steps
    prepare_line1();
    // start motors, non-blocking
    stepper_start_cycle();
}

void loop() {
    static int prevTime = 0;
    // Debug messages - print current positions of motors once per second
    // while they are rotating, once per 10 seconds when they are stopped
    int currTime = millis();
    if( (stepper_cycle_running() && (currTime - prevTime) >= 1000) || (currTime - prevTime) >= 10000 ) {
        prevTime = currTime;
        Serial.print("X.pos=");
        Serial.print(sm_x.current_pos, DEC);
        Serial.print(", Y.pos=");
        Serial.print(sm_y.current_pos, DEC);
        Serial.print(", Z.pos=");
        Serial.print(sm_z.current_pos, DEC);
        Serial.println();
    }
    
    // put any code here, it would run while the motors are rotating
}
```

even if you don't have stepper motor with step-dir driver, you can check out output in the serial monitor window (this one goes from my ChipKIT Uno32)

```
Starting stepper_h test...
X.pos=6937500, Y.pos=3525000, Z.pos=352500
X.pos=14437500, Y.pos=7275000, Z.pos=727500
X.pos=21930000, Y.pos=11025000, Z.pos=1102500
X.pos=29430000, Y.pos=14775000, Z.pos=1477500
X.pos=36922500, Y.pos=18525000, Z.pos=1852500
X.pos=44422500, Y.pos=22267500, Z.pos=2227500
X.pos=51915000, Y.pos=26017500, Z.pos=2602500
X.pos=59415000, Y.pos=29767500, Z.pos=2977500
X.pos=66907500, Y.pos=33517500, Z.pos=3352500
X.pos=74407500, Y.pos=37260000, Z.pos=3727500
X.pos=81900000, Y.pos=41010000, Z.pos=4102500
X.pos=89400000, Y.pos=44760000, Z.pos=4477500
X.pos=96892500, Y.pos=48510000, Z.pos=4852500
X.pos=104392500, Y.pos=52260000, Z.pos=5227500
X.pos=111885000, Y.pos=56010000, Z.pos=5602500
X.pos=119385000, Y.pos=59752500, Z.pos=5977500
X.pos=126877500, Y.pos=63502500, Z.pos=6352500
X.pos=134377500, Y.pos=67252500, Z.pos=6727500
X.pos=141870000, Y.pos=71002500, Z.pos=7102500
X.pos=149370000, Y.pos=74745000, Z.pos=7477500
X.pos=150000000, Y.pos=75000000, Z.pos=7500000
```

Actually, this lib has some more features like counting steps for each motor, tracking working tool virtual position, making steps with dynamic step delay (to draw curves) etc. Some of them are far from being finished, but some already work mostly fine. Will provide more examples if someone is interested here.

some older videos with older version of this stepper_h lib + ChipKIT + CNC

https://vimeo.com/133592759  
https://vimeo.com/93176233  
https://vimeo.com/93395529

---

# Note for example sketch compile error

Unfortunately, provided above example won't compile with upstream
Arduino (both Arduino and ChipKIT platforms) due to lack of support
of 64-bit values in Serial.println(xxx) functions:
Serial.println(int64_t, DEC) is missing

To fix this issue, copy patched 3pty/arduino/Print.cpp and 3pty/arduino/Print.h to:

for Arduino platform  
~/.arduino15/packages/arduino/hardware/avr/1.6.19/cores/arduino/

for ChipKIT platform  
~/.arduino15/packages/chipKIT/hardware/pic32/1.4.3/cores/pic32/

This is required to compile example sketch:

~~~cpp
    Serial.print(sm_x.current_pos, DEC);
~~~

(sm_x.current_pos has int64_t/"long long" data type)

Or remove/replace this line in example sketch,
patched Print is not required to compile stepper_h library core.

patched version of Print by
Rob Tillaart https://github.com/RobTillaart

Code to print int64_t and uint64_t for UNO (and maybe DUE)  
http://forum.arduino.cc/index.php/topic,143584.0.html  
https://github.com/arduino/Arduino/issues/1236

---
# Подключение моторов
https://github.com/1i7/stepper_h/issues/16  
https://github.com/1i7/stepper_h/issues/23

Для примеров используем
- Ардуино-совместимая микроконтроллерная плата ChipKIT Uno32 (чип PIC32MX, 80МГц)
- CNC-шилд Ардуино
- драйвер STEP-DIR Pololu-DRV8825 ("фиолетовый драйвер")
https://ru.aliexpress.com/item/20pcs-lot-StepStick-DRV8825-Stepper-Driver-Pololu-Reprap-4layer-PCB-Sanguinololu-RAMPS/32417447864.html?spm=a2g0v.10010108.1000016.1.38f7f7319jDHWL&isOrigTitle=true
- Шаговые моторы 17HS4401 (побольше; 1.7А 0.42N.M), 17HS3401 (cредний), 17HS2408 (самый маленький).

Посчитаем самые интересные граничные условия для аппаратных устройств и логики:
- минимальные задержки между шагами мотора для разных значений делителя шага (драйвер+мотор)
- минимальная частота таймера для одновременного вращения несколькими моторами с максимальной скоростью (драйвер+мотор+контроллер)
- максимальная скорость перемещения координаты для некоторых типовых передаточных механизмов

## Провода
Моторы подключаются к драйверу 4-жильным шлейфом. Так получилось, что порядок проводов на стороне мотора и на стороне драйвера отличается (хотя в купленном проводе они скорее всего будут идти по порядку - в этом случае их нужно переставить).

На стороне мотора должно быть: синий, красный, зеленый, черный.
На стороне драйвера при этом: синий, зеленый, черный, красный.

По умолчанию провода приехали с одинаковым порядком с обоих концов, поэтому управляющие сигналы на обмотки шли непонятным образом, драйвер грелся, мотор грелся, хотя и подавал признаки жизни.

Спасибо Игорю Герасимову за фикс, всё отлично крутится.

## Регулировка драйвера
Сюда же бонусом - правило регулировки драйвера под мотор

> Формула самая простая :
> 1) сила тока рабочая шагового мотора берётся
> 2) значение Vref = Current Limit / 2
> 
> собственно - сила тока поделить на Vref = вольты
> 
> берёшь мультиметр ставишь на измерение вольт (на 20) и измеряешь. Плюсовой щуп ставишь на резистор крутящийся, минусовой на минус соответственно. Самое главное не отсоединять мотор при включённом питании ИБП.
> 
> мы брали движки
> 1.7 ампера
> 0.6 ампера
> 1.3 ампера
> 
> вывод формулы :
> 1.7/2=0.85 вольта ( настроить резистор примерно 0.75 - 0.80 )
> 0.6/2=0.3 ( 0.2 - 0.25 )
> 1.3/2=0.65 ( 0.55 - 0.60 )
> 
> Мотор при работе не должен пищать жужжать и тп . 😊


## Делители шага

На мотор-шилде перемычки с настройками делителя шага помечены M0, M1, M2

Мотор 2-х фазный, полный оборот - 200 шагов.

Варианты делителей шага: 1/1 (без деления), 1/2, 1/4, 1/8, 1/16, 1/32

позиции перемычек для разных делителей:

- 1/1: M0=0, M1=0, M2=0 (000)
- 1/2: M0=1, M1=0, M2=0 (100)
- 1/4: M0=0, M1=1, M2=0 (010)
- 1/8: M0=1, M1=1, M2=0 (110)
- 1/16: M0=0, M1=0, M2=1 (001)
- 1/32: M0=1, M1=1, M2=1 (111)

## Минимальная задержка между шагами.

По результатам тестов выделяю 4 основные состояния мотора в зависимости от выставленной задержки:  
https://github.com/1i7/stepper_h/blob/master/test-sketches/step_dir_test/step_dir_test.ino

- мотор пищит и не крутится
- мотор гудит и не крутится
- мотор крутится, но если остановить его пальцем, сам движение не продолжает
- мотор крутится и если остановить его пальцем, сам продолжает движение

Чем меньше делитель шага, тем более размыта граница между состояниями. Например, пограничное состояние "если остановить его пальцем, сам движение не продолжает" иногда воспроизводится стабильно, а иногда мотор может взять и продолжить. Если "мотор гудит и не крутится", то можно его немного крутануть пальцем и он начинает крутится. Или в режиме без деления при задержке 600 микросекунд мотор вроде как пищит и даже при попытке крутануть пальцем не начинает вращаться, но если плавно провернуть его 3-4 раза, то все равно начинает вращение.

Скорее всего режимы "сам движение не продолжает" можно будет включить в рабочий диапазн в том случае, если стартовать мотор не сразу на максимальной скорости, а применить к нему плавное ускорение. Сопротивление при вращении мотор все равно оказывает, стопор может произойти только при полной остановке.

Так же, скорее всего, границы состояний будут плавать при разных настройках драйвера (все текущие тесты производились в одном положении регулятора).

Оптимальные задержки между шагами для разных делителей (крутится ок, если притормозить, то продолжает полюбому):
- 1/1: 1500 мкс
- 1/2: 650 мкс
- 1/4: 330 мкс
- 1/8: 180 мкс
- 1/16: 80 мкс
- 1/32: 40 мкс

## Максимальная скорость вращения

Посчитаем максимальные скорости вращения  
https://github.com/1i7/stepper_h/blob/master/test-sketches/step_dir_test/step_dir_test.ino  
https://github.com/1i7/stepper_h/blob/master/test-sketches/stepper_modes/stepper_modes.ino

- 1/1: 200 шагов/оборот
- 1/2: 200 * 2 = 400 шагов/оборот
- 1/4: 200 * 4 = 800 шагов/оборот
- 1/8: 200 * 8 = 1600 шагов/оборот
- 1/16: 200 * 16 = 3200 шагов/оборот
- 1/32: 300 * 32 = 6400 шагов/оборот

Скорости
- 1/1: 1500мкс/шаг (оптимальный стабильный вариант)  
1500мкс/шаг*200шагов/оборот=300000мкс/об=300млс/об=0.3с/об  
макс.ск = 1/0.3об/сек = **3.3 об/сек**

- 1/1: 1000мкс/шаг (крутит с толчка)  
1000мкс/шаг*200шагов/оборот=200000мкс/об=200млс/об=0.2с/об  
макс.ск=1/0.2 = **5 об/сек**

- 1/2: 650мкс/шаг (стабильный вариант)  
650мкс/шаг*400шагов/оборот=260000мкс/об=260млс/об=0.260с/об  
макс.ск=1/0.260об/с = **3.8 об/сек**

- 1/4: 330мкс/шаг (стабильный вариант)  
330мкс/шаг*800шагов/оборот=264000мкс/об=264млс/об=0.264с/об  
макс.ск=1/0.264об/с = **3.8 об/сек**

- 1/8: 180мкс/шаг (стабильный вариант)  
180мкс/шаг*1600шагов/оборот=288000мкс/об=288млс/об=0.288с/об  
макс.ск=1/0.288об/с = **3.8 об/сек**

- 1/16: 80мкс/шаг (стабильный вариант)  
80мкс/шаг*3200шагов/оборот=256000мкс/об=256млс/об=0.256с/об  
макс.ск=1/0.256об/с = **3.9 об/сек**

- 1/32: 60мкс/шаг (медленнее, чем лучший стабильный вариант, но держит 3 мотора на ChipKIT с PIC32MX)  
60мкс/шаг*6400шагов/оборот=384000мкс/об=384млс/об=0.384с/об  
макс.ск=1/0.384об/сек = **2.6 об/сек**

- 1/32: 40мкс/шаг (стабильный вариант, 2 мотора на ChipKIT с PIC32MX)  
40мкс/шаг*6400шагов/оборот=256000мкс/об=256млс/об=0.256с/об  
макс.ск=1/0.256об/с = **3.9 об/сек**

- 1/32: 30 мкс/шаг (чуть менее стабильный, чем 40 - самый быстрый из рабочих)  
30мкс/шаг*6400шагов/оборот=192000мкс/об=192млс/об=0.192с/об  
макс.ск=1/0.192об/сек = **5.2 об/сек**

В итоге получили, что примерная скорость на стабильных режимах на всех делителях получается в районе ~4об/сек.

## Линейная скорость координаты
Посчитаем длину шага и линейную скорость координаты для ременной передачи.

Берем шкив GT2:
- 20 зубов
- шаг зуба 2мм (по словам Игоря)

типа такого
https://ru.aliexpress.com/item/2GT-timing-belt-wheel-16-teeth-20-teeth-width-6mm-inner-hole-5-8mm-for-DIY/32728094223.html?spm=a2g0v.10010108.1000016.1.fb88affc9Zvzz&isOrigTitle=true

По логике вещей полная длина окружности шкива будет
L=2мм*20зубов=40мм

т.е. за полный оборот мотор переместит 40мм (=40000мкм) длины ремня.

Длина шага будет:
- 1/1: 200 шагов/оборот, шаг=40000мкм/200 = **200 мкм** = 200000 нм
- 1/2: 200 * 2 = 400 шагов/оборот, шаг=40000мкм/400 = **100 мкм** = 100000 нм
- 1/4: 200 * 4 = 800 шагов/оборот, шаг=40000мкм/800 = **50 мкм** = 50000 нм
- 1/8: 200 * 8 = 1600 шагов/оборот, шаг=40000мкм/1600 = **25 мкм** = 25000 нм
- 1/16: 200 * 16 = 3200 шагов/оборот, шаг=40000мкм/3200 = **12.5 мкм** = 12500 нм
- 1/32: 300 * 32 = 6400 шагов/оборот, шаг=40000мкм/6400 = **6.25 мкм** = 6250 нм


Некоторые варианты скорости (для наглядности габаритов: A4:210×297 мм)

- 1/1: 1500мкс/шаг (оптимальный стабильный вариант)
макс.ск = 3.3 об/сек
макс.ск (шкив) = 40000мкм/об * 3.3об/с= 132000мкм/с = **132 мм/с** = 13.2 см/с

- 1/2: 650мкс/шаг (стабильный вариант)
макс.ск = 3.8 об/сек
макс.ск (шкив) = 40000мкм/об * 3.8об/с = 152000мкм/с = **152 мм/с** = 15.2 см/с

- 1/4: 330мкс/шаг (стабильный вариант)
макс.ск = 3.8 об/сек
макс.ск (шкив) = 40000мкм/об * 3.8об/с = 152000мкм/с = **152 мм/с** = 15.2 см/с

- 1/8: 180мкс/шаг (стабильный вариант)
макс.ск = 3.8 об/сек
макс.ск (шкив) = 40000мкм/об * 3.8об/с = 152000мкм/с = **152 мм/с** = 15.2 см/с

- 1/16: 80мкс/шаг (стабильный вариант)
макс.ск = 3.9 об/сек
мкс.ск (шкив) = 40000мкм/об * 3.9об/с = 156000мкм/с = **156 мм/с** = 15.6 см/с

- 1/32: 60мкс/шаг (медленнее, чем лучший стабильный вариант, но держит 3 мотора на ChipKIT с PIC32MX)
макс.ск = 2.6 об/сек
макс.ск (шкив) = 40000мкм/об * 2.6об/с= 104000мкм/с = **104 мм/с** = 10.4 см/с

- 1/32: 40мкс/шаг (стабильный вариант, 2 мотора на ChipKIT с PIC32MX)
макс.ск = 3.9 об/сек
мкс.ск (шкив) = 40000мкм/об * 3.9об/с = 156000мкм/с = **156 мм/с** = 15.6 см/с

- 1/32: 30мкс/шаг (чуть менее стабильный, чем 40 - самый быстрый из рабочих)
макс.ск = 5.2 об/сек
макс.ск (шкив) = 40000мкм/об * 5.2об/с= 208000мкм/с = **208 мм/с** = 20.8 см/с

~~~cpp
// 1/1, 132 mm/s
int _step_delay_us = 1500; // us
int _dist_per_step = 200000; // nm
init_stepper(&sm_x, 'x', STEP_PIN, DIR_PIN, EN_PIN, false, _step_delay_us, _dist_per_step);

// 1/2, 152 mm/s
int _step_delay_us = 650; // us
int _dist_per_step = 100000; // nm
init_stepper(&sm_x, 'x', STEP_PIN, DIR_PIN, EN_PIN, false, _step_delay_us, _dist_per_step);

// 1/4, 152 mm/s
int _step_delay_us = 330; // us
int _dist_per_step = 50000; // nm
init_stepper(&sm_x, 'x', STEP_PIN, DIR_PIN, EN_PIN, false, _step_delay_us, _dist_per_step);

// 1/8, 152 mm/s
int step_delay_us = 180; // us
int dist_per_step = 25000; // nm
init_stepper(&sm_x, 'x', STEP_PIN, DIR_PIN, EN_PIN, false, _step_delay_us, _dist_per_step);

// 1/16, 156 mm/s
int step_delay_us = 80; // us
int dist_per_step = 12500; // nm
init_stepper(&sm_x, 'x', STEP_PIN, DIR_PIN, EN_PIN, false, _step_delay_us, _dist_per_step);

// 1/32, 104 mm/s
int step_delay_us = 60; // us
int dist_per_step = 6250; // nm
init_stepper(&sm_x, 'x', STEP_PIN, DIR_PIN, EN_PIN, false, _step_delay_us, _dist_per_step);

// 1/32, 156 mm/s
int step_delay_us = 40; // us
int dist_per_step = 6250; // nm
init_stepper(&sm_x, 'x', STEP_PIN, DIR_PIN, EN_PIN, false, _step_delay_us, _dist_per_step);
~~~

---
# Про базовую единицу измерения и размеры рабочей области

Единица измерения выбирается в зависимости от задачи и свойств передаточного механизма (проще всего считать за нанометры).

Тип данных curren_pos, min_pos и max_pos - long long (int64_t), 64-битное знаковое целое.

Для 64-битного значения current_pos размеры рабочей области с базовой единицей нанометры:  
2^63=9223372036854776000 нанометров /1000/1000/1000 = 9223372037 метров /1000 = 9223372км (9 миллионов км).  
в обе стороны от -9млн км до 9млн км, всего 18млн км (1/3 пути до Марса)

64-битные типы данных не поддерживаются аппаратно на 32-битных
(тем более, на 16-битных) контроллерах, но они реализованы на уровне
компилятора и библиотеки libc (как минимум, для платформ ChipKIT и Arduino).
Они могу работать чуть медленнее, чем "родные" (на 32-битных контроллерах)
32-битные переменные long, но потеря производительности по факту
оказывается не существенной даже в критических частях кода
(сравнение с точностью до микросекунд не показало разницы).

При этом использование 64-битных значений фактически позволяет
не задумываться о максимальных границах рабочей области.

Для 32хбитного значения current_pos размеры рабочей области были бы:

- Если брать базовую единицу измерения за нанометры (1/1000 микрометра),
то диапазон значений для рабочей области будет от нуля в одну сторону:  
  2^31=2147483648-1 нанометров/1000/1000/1000=2.15метра  
в обе строны: [-2.15м, 2.15м], т.е. всего 4.3 метра.

- Для базовой единицы микрометр (микрон) рабочая область  
от -2.15км до 2.15км, всего 4.3км.

Для 32-битного случая вариант рабочей области 4.3 метра (2.15, если считать от 0)
с нанометрами для многих случаев в принципе приемлем, но почти не оставляет запаса
для экспериментов.

Вариант размера рабочей области с базовой идиницей микрометры более, чем
достаточен, но размер шага для настольных станков (хотя они на уровне механики
могут не поддерживать такую точность) математически часто предполагает доли
микрон (6.15мкм, 7.5мкм и т.п.), поэтому в качестве целевой единицы измерения
рекомендуется ориентироваться на целочисленные нанометры.

---
# Максимальное количество шагов за один цикл prepare_steps

На фиолетовом драйвере с делителем шага 32 мотор будет стабильно работать при минимальной задержке между шагами step_delay=30 микросекунд (ок, самый минимальный вариант - 20мкс, но он уже не очень стабильный; 10 микросекунд - мотор просто гудит). На PIC32MX (ChipKIT Uno32) с библиотекой stepper_h с такой задержкой можно крутить одновременно 2 мотора.

для количества шагов step_count знаковое целое (long) можно на один цикл задать максимальное количество шагов:  
2^31=2147483648

30мкс*2147483648шагов = 64424509440мкс/1000=64424509секунд/60=1073741824мин/60= 17895697часов/24=745654дней/365=2042 лет

https://duckduckgo.com/?q=2%5E31*30%2F60%2F60%2F24%2F365&t=canonical&atb=v76-1&ia=calculator

пожалуй, хватит на цикл без int64_t

для step_delay=20 микросекунд (прям совсем максимальная скорость на китайском бросовом железе)  
2^31*20/60/60/24/365=1361 год

https://duckduckgo.com/?q=2%5E31*20%2F60%2F60%2F24%2F365&t=canonical&atb=v76-1&ia=calculator

чуть поменьше, но, наверное, тоже достаточно

Для полноты картины, если заменить long (32 бит) на int (16 бит на Ардуине), получим:  
2^15*20/60/60/24=7.5 дней.

не так круто, как long, но тоже ничего, но пусть будет long

---

# Внутреннее устройство

Код обработчика таймера handle_interrupts  
https://github.com/1i7/stepper_h/blob/master/stepper_h/stepper_timer.cpp#L824

на одном шаге работает в 3 приема:
- первый проход (за 2 импульса до шага) - проверяет границы
- второй проход (за 1 импульс до шага) - взводит ножку step на HIGH
- третий проход (ипульс шага) - делает шаг (сбрасывает step в LOW)

на третьем же проходе сразу происходят проверки, нужно ли делать следующий шаг.

