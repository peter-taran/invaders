#pragma once

// В файле содержится всё необходимое для работы с физическим (не календарным) временем,
// измеряемым при помощи имеющегося в системе точного счётчика. При отсутствии счётчика
// автоматически переключаемся на менее точный GetTickCount
// 
// Время может измеряться в разных единицах:
//   - ticks: LONGLONG, в тиках счётчика
//   - msec:  LONG, целым числом милисекунд
//   - 100ns: LONG, 100-наносекундными интервалами, единица активно используется Виндой
//   - sec:   double, в секундах, в том числе дробных


// Для работы 64-битных Interlocked-функций надо, чтобы переменные были выровнены
// на границу 64 бит, т.е. pragma pack не должен быть меньше 8
#pragma pack(push, 8)

namespace timers
{

// Маркет неинициализированного значения
static const struct Uninitialized {} uninitialized;

// Переводит тики счётчика в другие единицы измерения и обратно
// После конструирования может использоваться многопоточно
class TicksCvt
{
public:
	// Тут запрашивается частота работы счётчика
	TicksCvt() throw();
	
	// Тики счётчика -> секунды
	double toSec(const LONGLONG& ticks) const throw();
	
	// Секунды -> тики счётчика
	LONGLONG fromSec(double secs) const throw();

	// Тики счётчика -> милисекунды
	LONG toMsec(const LONGLONG& ticks) const throw();
	
	// Милисекунды -> тики счётчика
	LONGLONG fromMsec(LONG msecs) const throw();

	// Тики счётчика -> 100-наносекундные интервалы (в них меряется файловое время)
	LONG to100ns(const LONGLONG& ticks) const throw();
	
	// 100-наносекундные интервалы -> тики счётчика
	LONGLONG from100ns(LONG timeIn100ns) const throw();

private:
	double _secsPersTicks; // тиков счётчика в секунду
};


// Возвращает текущий момент времени, в тиках счётчика
LONGLONG now() throw();

// Умеет возвращать текущий момент времени в разных единицах измерения
// timers::Now g_now;
// void foo()
// {
//   while(condition) {
//     // ...
//     cout << "time: " g_now.msec() << endl;
//   };
// };
class Now
{
public:
	// текущий момент в тиках счётчика, эквивалент глобальной функции now()
	static LONGLONG ticks() throw();
	
	// в других единицах измерения
	LONG msec() const throw();
	LONG in100ns() const throw();
	double sec() const throw();

private:
	TicksCvt _converter;
};


// Пропустить заданный промежуток времени (в разных единицах измерения)
// Аналогичное делает системная функция Sleep, но эти работают точно
void SleepPrecTicks(LONGLONG sleepTimeInTicks) throw();
void SleepPrecMs(LONG sleepTimeInMsec) throw();
void SleepPrecSec(double sleepTimeInSec) throw();


// Класс для измерения промежутков времени
// Возвращает, сколько времени прошло с момента конструирования класса
// или с последнего вызова reset
// Многопоточность не поддерживается, расчитан на использование в одном потоке
class Timer
{
public:
	// Отсчёт времени начинается с момента конструирования
	Timer() throw();
	
	// Специальный вариант, когда таймер не инициализирован, остаётся мусор,
	// когда надо будет начать зовите reset
	explicit Timer(const Uninitialized&) throw();
	
	// Копировать можно на здоровье
	
	// Начать отсчёт с настоящего момента
	void reset() throw();

	// Прошедшее время в тиках счётчика
	LONGLONG passed() const throw();
	
	// Вернуть прошедшее время (в тиках) и сбросить счетчик
	// Полезно, когда один счётчик измеряет одну за другой стадии одного процесса
	// Фактически, это объединённые passed и reset
	LONGLONG stage() throw();
	
private:
	LONGLONG _start;
};


// Таймер со встроенным конвертором, сразу умеет возвращать время в других
// единицах измерения
class TimerEx: public Timer
{
public:
	// Прошедшее время в других единицах измерения
	double passedSec() const throw();
	LONG passedMsec() const throw();
	LONG passed100ns() const throw();
	
private:
	TicksCvt _converter;
};


// Аккумулятор времени
// Вызовами оператора += добавляет к имеющемуся времени новый промежуток
// -После конструирования может использоваться многопоточно-
// Многопоточность не поддерживается, синхронизируйте снаружи
class TimeAccum
{
public:
	// Изначально промежуток времени нулевой
	TimeAccum() throw();
	
	// Сбросить счётчик в ноль, вернув текущее значение (в тиках)
	LONGLONG clear() throw();
	
	// Копировать можно
	TimeAccum(const TimeAccum& from) throw();
	TimeAccum& operator=(const TimeAccum& from) throw();
	
	// Добавить новый прошедший промежуток времени (в тиках счётчика)
	TimeAccum& operator+=(const LONGLONG& passedTimeInTicks) throw();
	TimeAccum& operator+=(const Timer& timer) throw();
	
	// Добавить прошедшее по таймеру время и сбросить (перезапустить) таймер
	void addAndReset(Timer& timer) throw();
	
	// Текущее значение накопленного времени
	LONGLONG value() const throw();
	
private:
	mutable LONGLONG _value;
};


// Аккумулятор времени со встроенным преобразователем времени в другие единицы
// измерения
class TimeAccumEx: public TimeAccum
{
public:
	// Накопленное время в секундах и милисекундах
	// (при множественных вызовах работает неоптимально, каждый раз запрашивается
	// частота счётчика)
	double valueSec() const throw();
	LONG valueMsec() const throw();
	LONG value100ns() const throw();

private:
	TicksCvt _converter;
};


// диапазон времени
// всё измеряется в тиках счётчика
struct TimeRange
{
	// конструктор заполняет нулями
	// (НЕ ТЕКУЩИМ МОМЕНТОМ!)
	TimeRange() throw();
	
	LONGLONG start; // начало диапазона
	LONGLONG end;   // конец
	
	LONGLONG length() const throw(); // длина временного промежутка
};


} // namespace timers

#pragma pack(pop)
