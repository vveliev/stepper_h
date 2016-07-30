/**
 * stepper.h
 *
 * Библиотека управления шаговыми моторами, подключенными через интерфейс 
 * драйвера "step-dir".
 *
 * LGPL, 2014
 *
 * @author Антон Моисеев
 */


#ifndef STEPPER_H
#define STEPPER_H

/**
 * Стратегия определения границы движения координаты в одном из направлений:
 * - CONST: значение координаты задается константой в настройках мотора (min/max _pos)
 * - AUTO: использовать калибровку для определения конечной позиции в этом направлении 
 *       (значение min/max _pos обновляется после калибровки; после калибровки аналогично CONST)
 * - INF: не органичивать движение координаты в этом направлении (значение min/max _pos игнорируется)
 */
typedef enum {CONST, AUTO, INF} end_strategy_t;

/**
 * Режим цикла вращения мотора
 */
typedef enum {
    /** Режим калибровки выключен */
    NONE,
    
    /** Калибровка начальной позиции (сбрасывать current_pos в min_pos при каждом шаге) */
    CALIBRATE_START_MIN_POS, 
    
    /** Калибровка границ рабочей области (сбрасывать max_pos в current_pos при каждом шаге) */
    CALIBRATE_BOUNDS_MAX_POS
} calibrate_mode_t;

/**
 * Структура - шаговый двигатель.
 */
typedef struct {
    /** 
     * Имя шагового мотора (один символ: X, Y, Z и т.п.) 
     */
    char name;
    
    /*************************************************************/
    /* Подключение мотора к драйверу step-dir */
    /*************************************************************/
    
    /* Информация о подключение через драйвер Step-dir */
    
    /** 
     * Подача периодического импульса HIGH/LOW будет вращать мотор 
     */
    int pin_step;
    
    /** 
     * Направление вращения
     * 1 (HIGH): в одну сторону
     * 0 (LOW): в другую
     *
     * Для движения в сторону увеличения значения виртуальной координаты: 
     * при dir_inv=1: запись 1 (HIGH) в pin_dir
     * при dir_inv=-1: запись 0 (LOW) в pin_dir
     */
    int pin_dir;
    
    /** 
     * Вкл (0)/выкл (1) мотор 
     */
    int pin_en;
    
    /*************************************************************/
    /* Концевые датчики */
    /*************************************************************/
    
    /** 
     * Датчик на конце минимального положения текущей координаты;
     * -1: датчик не подключен
     */
    int pin_min;
    
    /** 
     * Датчик на конце максимального положения текущей координаты;
     * -1: датчик не подключен
     */
    int pin_max;
    
    /*************************************************************/
    /* Настройки подключения - характеристики мотора, драйвера и привода */
    /*************************************************************/
    
    /**
     * Инверсия направления вращения.
     *
     * Для движения в сторону увеличения значения виртуальной координаты: 
     * при dir_inv=1: запись 1 (HIGH) в pin_dir
     * при dir_inv=-1: запись 0 (LOW) в pin_dir
     */
    int dir_inv;
    
    /** 
     * Минимальная задержка между импульсами, микросекунды 
     * (для движения с максимальной скоростью) 
     */
    int pulse_delay;
    
    /** 
     * Расстояние, проходимое координатой за шаг, микрометры
     * (на основе значения distance_per_step счетчик шагов вычисляет
     * текущее положение рабочей координаты)
     */
    float distance_per_step;
    
    /*************************************************************/
    /* Характеристики рабочей области */
    /*************************************************************/
    
    /**
     * Стратегия определения конечного положения для минимальной позиции координаты:
     * CONST/AUTO/INF
     */
    end_strategy_t min_end_strategy;
    
    /**
     * Стратегия определения конечного положения для максимальной позиции координаты:
     * CONST/AUTO/INF
     */
    end_strategy_t max_end_strategy;
    
    /** 
     * Минимальное значение положения координаты, микрометры 
     */
    float min_pos;
    
    /** 
     * Максимальное значение положения координаты, микрометры 
     */
    float max_pos;
    
    /*************************************************************/
    /* Информация о движении координаты, подключенной к мотору */
    /*************************************************************/
    
    /** 
     * Текущее положение координаты, микрометры 
     * (вычисляется и обновляется программно счетчиком шагов 
     * на основе значения distance_per_step).
     * 
     * При dir=1 координата возрастает, при dir=0 координата убывает
     */
    float current_pos;
} stepper;


/**
 * Инициализировать шаговый мотор необходимыми значениями.
 */
void init_stepper(stepper* smotor,  char name, 
        int pin_step, int pin_dir, int pin_en,
        int dir_inv, int pulse_delay,
        double distance_per_step);
        
/**
 * Задать настройки границ рабочей области для шагового мотора.
 * 
 * Примеры:
 * 1) область с заранее известными границами:
 *   init_stepper_ends(&sm_z, -1, -1, CONST, CONST, 0, 100000);
 * 
 * калибровка текущей позиции curr_pos устанавливает начальное значение в min_pos,
 * калибровка ширины области не требуется 
 *
 * 2) область с заранее известной позицией min_pos, значение max_pos (задаёт ширину области) 
 * определяется калибровкой:
 *   init_stepper_ends(&sm_z, -1, -1, CONST, AUTO, 0, 100000);
 *
 * калибровка текущей позиции curr_pos устанавливает начальное значение в min_pos,
 * калибровка ширины области задает новое значение max_pos (до калибровки используется
 * значение по умолчанию)
 *
 */
void init_stepper_ends(stepper* smotor,
        int pin_min, int pin_max,
        end_strategy_t min_end_strategy, end_strategy_t max_end_strategy,
        double min_pos, double max_pos);
        
/**
 * Подготовить мотор к запуску ограниченной серии шагов - задать нужное количество 
 * шагов и задержку между шагами для регулирования скорости (0 для максимальной скорости).
 * 
 * @param step_count количество шагов, знак задает направление вращения
 * @param step_delay задержка между двумя шагами, микросекунды (0 для максимальной скорости).
 */
void prepare_steps(stepper *smotor, int step_count, int step_delay);

/**
 * Подготовить мотор к запуску на вращение - задать направление и задержку между
 * шагами для регулирования скорости (0 для максимальной скорости).
 *
 * @param dir направление вращения: 1 - вращать вперед, -1 - назад.
 * @param step_delay задержка между двумя шагами, микросекунды (0 для максимальной скорости).
 * @param calibrate_mode - режим калибровки
 *     NONE: режим калибровки выключен - останавливать вращение при выходе за виртуальные границы 
 *           рабочей области [min_pos, max_pos] (аппаратные проверяются ВСЕГДА);
 *     CALIBRATE_START_MIN_POS: установка начальной позиции (сбрасывать current_pos в min_pos при каждом шаге);
 *     CALIBRATE_BOUNDS_MAX_POS: установка размеров рабочей области (сбрасывать max_pos в current_pos при каждом шаге).
 */
void prepare_whirl(stepper *smotor, int dir, int step_delay, calibrate_mode_t calibrate_mode);

/**
 * Подготовить мотор к запуску ограниченной серии шагов с переменной скоростью - задержки на каждом 
 * шаге вычисляются заранее, передаются в буфере delay_buffer.
 * 
 * Масштабирование шага позволяет экономить место в буфере delay_buffer, жертвуя точностью 
 * (минимальной длиной шага в цикле); если цикл содержит серии шагов с одинаковой задержкой,
 * реальноая точность не пострадает. Буфер delay_buffer содержит временные задержки перед каждым следующим шагом.
 * Можно использовать одну и ту же задержку (один элемент буфера) для нескольких последовательных шагов
 * при помощи параметра scale (масштаб). 
 * 
 * При scale=1 на каждый элемент буфера delay_buffer ("виртуальный" шаг) мотор будет делать 
 *     один реальный (аппаратный) шаг из delay_buffer.
 * При scale=2 на каждый элемент буфера delay_buffer (виртуальный шаг) мотор будет делать 
 *     два реальных (аппаратных) шага с одной и той же задержкой из delay_buffer.
 * При scale=3 на каждый элемент буфера delay_buffer (виртуальный шаг) мотор будет делать 
 *     три реальных (аппаратных) шага с одной и той же задержкой из delay_buffer.
 * 
 * Допустим, в delay_buffer 2 элемента (2 виртуальных шага):
 *     delay_buffer[0]=1000
 *     delay_buffer[1]=2000
 * параметр scale=3
 * 
 * Мотор сделает 3 аппаратных шага с задержкой delay_buffer[0]=1000 мкс перед каждым шагом и 
 * 3 аппаратных шага с задержкой delay_buffer[1]=2000мкс. Всего 2*3=6 аппаратных шагов, 
 * время на все шаги = 1000*3+2000*3=3000+6000=9000мкс
 * 
 * Значение параметра step_count указываем 2 (количество элементов в буфере delay_buffer).
 *
 * Аналогичный результат можно достигнуть с delay_buffer[6]
 *     delay_buffer[0]=1000
 *     delay_buffer[1]=1000
 *     delay_buffer[2]=1000
 *     delay_buffer[3]=2000
 *     delay_buffer[4]=2000
 *     delay_buffer[5]=2000
 * scale=1, step_count=6
 *
 * Количество аппаратных шагов можно вычислять как step_count*scale.
 * 
 * @param step_count количество элементов в буфере delay_buffer (количество виртуальных шагов), 
 *     знак задает направление вращения мотора.
 * @param delay_buffer - массив задержек перед каждым следующим шагом, микросекунды
 * @param scale масштабирование шага - количество аппаратных шагов мотора в одном 
 *     виртуальном шаге
 * Значение по умолчанию scale=1: виртуальные шаги соответствуют аппаратным
 */
void prepare_buffered_steps(stepper *smotor, int step_count, int* delay_buffer, int scale=1);

/**
 * Подготовить мотор к запуску ограниченной серии шагов с переменной скоростью - задать нужное количество 
 * шагов и указатель на функцию, вычисляющую задержку перед каждым шагом для регулирования скорости.
 * 
 * @param step_count количество шагов, знак задает направление вращения
 * @param curve_context - указатель на объект, содержащий всю необходимую информацию для вычисления
 *     времени до следующего шага
 * @param next_step_delay указатель на функцию, вычисляющую задержка перед следующим шагом, микросекунды
 */
void prepare_curved_steps(stepper *smotor, int step_count, void* curve_context, int (*next_step_delay)(int curr_step, void* curve_context));

/**
 * Запустить цикл шагов на выполнение - запускаем таймер, обработчик прерываний
 * отрабатывать подготовленную программу.
 */
void start_stepper_cycle();

/**
 * Завершить цикл шагов - остановить таймер, обнулить список моторов.
 */
void finish_stepper_cycle();

/**
 * Текущий статус цикла:
 * true - в процессе выполнения,
 * false - ожидает.
 */
bool is_cycle_running();

/**
 * Отладочная информация о текущем цикле.
 */
void cycle_status(char* status_str);

////
// Математика

/**
 * Подготовить линейное перемещение из текущей позиции в заданную точку с заданной скоростью,
 * для одной координаты.
 *
 * @param sm - мотор на выбранной координате
 * @param dl - сдвиг по указанной оси, мм
 * @param spd - скорость перемещения, мм/с, 0 для максимальное скорости
 * 
 */
void prepare_line(stepper *sm, double dl, double spd=0);

/**
 * Подготовить линейное перемещение из текущей позиции в заданную точку с заданной скоростью,
 * для двух координат.
 *
 * @param dl1 - сдвиг по оси 1, мм
 * @param dl2 - сдвиг по оси 2, мм
 * @param spd - скорость перемещения, мм/с, 0 для максимальное скорости
 */
void prepare_line_2d(stepper *sm1, stepper *sm2, double dl1, double dl2, double spd=0);


void prepare_circle(stepper *sm1, stepper *sm2, double center_c1, double center_c2, double spd);

void prepare_spiral_circle(stepper *sm1, stepper *sm2, stepper *sm3,double target_c3, double center_c1, double center_c2, double spd);

/**
 * @param target_c1 - целевое значение координаты 1, мм
 * @param target_c2 - целевое значение координаты 2, мм
 * @param spd - скорость перемещения, мм/с, 0 для максимальное скорости
 */
void prepare_arc(stepper *sm1, stepper *sm2, double target_c1, double target_c2, double center_c1, double center_c2, double spd);

void prepare_spiral_arc(stepper *sm1, stepper *sm2, stepper *sm3, double target_c1, double target_c2, double target_c3, double center_c1, double center_c2, double spd);

/**
 * @param target_c1 - целевое значение координаты 1, мм
 * @param target_c2 - целевое значение координаты 2, мм
 * @param spd - скорость перемещения, мм/с, 0 для максимальное скорости
 */
void prepare_arc2(stepper *sm1, stepper *sm2, double target_c1, double target_c2, double radius, double spd);

void prepare_spiral_arc2(stepper *sm1, stepper *sm2, stepper *sm3, double target_c1, double target_c2, double target_c3, double radius, double spd);

#endif // STEPPER_H



