#include "stdafx.h"

#pragma pack(push, 8)

namespace timers
{
    
// [19.10.2012 tarkil] отмечалась проблема с тем, что на многопроцессорных машинах
// последующие вызовы QueryPerformanceCounter могут дать меньше значения, чем
// предыдущие, это бывает из-за бага BIOS или HAL.
// См. http://msdn.microsoft.com/en-us/library/ms644904(VS.85).aspx?ppud=4
// У нас случилось на \\zenith-co, работающем под VirtualBox
// Поэтому последнее возвращённое значение сохраняется и каждое новое значение таймера
// проверяется, чтобы оно не было меньше с предыдущим значением.
// Да, страдает и точность и скорость, но по крайней мере время не идёт назад
// Закомментарьте макрос, если обход бага нужно отключить
// TODO: в исправлении используется слегка сомнительный __declspec(thread), если без него,
// то значение общее на все потоки и по факту счётчик в одних потоках может обгонять
// другие; если вообще без коррекции, значения кривеют (или нет - непонятно)
//#define TIMERS_FIX_PERF_COUNTER_BUG


#ifdef TIMERS_FIX_PERF_COUNTER_BUG
	__declspec(selectany) __declspec(thread) LONGLONG LAST_PERF_COUNTER = 0;
#endif

LONGLONG now()
{
	LARGE_INTEGER result;
	if( QueryPerformanceCounter(&result) )
	{
		#ifdef TIMERS_FIX_PERF_COUNTER_BUG
			LONGLONG prev = InterlockedExchange64(&LAST_PERF_COUNTER, result.QuadPart);
			if( prev > result.QuadPart )
			{
				result.QuadPart = prev + 1;
				for(;;)
				{
					LONGLONG prev2 = InterlockedCompareExchange64(&LAST_PERF_COUNTER,
							result.QuadPart, prev);
					if( prev2 == prev || prev2 >= result.QuadPart )
						break;
					prev = prev2;
				}
			}
		#endif
	}
	else
	{
		result.QuadPart = LONGLONG(GetTickCount()) * 100000;
			// умножаем на 100000, чтобы интервалы можно было задать точные
			// интервалы даже при отсутствии точного счётчика, мс - грубовато
	}
	return result.QuadPart;
}

TicksCvt::TicksCvt()
{
	LARGE_INTEGER ratio;
	if( QueryPerformanceFrequency(&ratio) )
		_secsPersTicks = 1.0 / ratio.QuadPart;
	else
		_secsPersTicks = 0.00000001; // для GetTickCount: 1000 * 100000 тиков/сек
}

double TicksCvt::toSec(const LONGLONG& ticks) const
{
	return _secsPersTicks * ticks;
}

LONG TicksCvt::toMsec(const LONGLONG& ticks) const
{
	return LONG((_secsPersTicks * ticks * 1000) + 0.5);
}

LONG TicksCvt::to100ns(const LONGLONG& ticks) const
{
	return LONG((_secsPersTicks * ticks * 10000000) + 0.5);
}

LONGLONG TicksCvt::fromMsec(LONG msecs) const
{
	return LONGLONG(double(msecs) / 1000 / _secsPersTicks);
}

LONGLONG TicksCvt::from100ns(LONG timeIn100ns) const
{
	return LONGLONG(double(timeIn100ns) / 10000000 / _secsPersTicks);
}

LONGLONG TicksCvt::fromSec(double secs) const
{
	return LONGLONG(secs / _secsPersTicks);
}

LONGLONG Now::ticks()
{
	return now();
}

LONG Now::msec() const
{
	return _converter.toMsec(now());
}

LONG Now::in100ns() const
{
	return _converter.to100ns(now());
}

double Now::sec() const
{
	return _converter.toSec(now());
}

Timer::Timer():
	_start(now())
{}

Timer::Timer(const Uninitialized&)
{}

void Timer::reset()
{
	_start = now();
}

LONGLONG Timer::passed() const
{
	return now() - _start;
}

LONGLONG Timer::stage()
{
	LONGLONG moment = now();
	LONGLONG result = moment - _start;
	_start = moment;
	return result;
}

double TimerEx::passedSec() const
{
	return _converter.toSec(passed());
}

LONG TimerEx::passedMsec() const
{
	return _converter.toMsec(passed());
}

LONG TimerEx::passed100ns() const
{
	return _converter.to100ns(passed());
}

TimeAccum::TimeAccum():
	_value(0)
{}

LONGLONG TimeAccum::clear()
{
	LONGLONG result = 0;
	std::swap(result, _value);
	return result;
}

TimeAccum::TimeAccum(const TimeAccum& from):
	_value(from._value)
{}

TimeAccum& TimeAccum::operator=(const TimeAccum& from)
{
	if( this != &from )
	{
		// так многопоточно, но не работает в WinXP: InterlockedExchange64(&_value, from._value);
		_value = from._value;
	}
	return *this;
}

TimeAccum& TimeAccum::operator+=(const LONGLONG& passedTimeInTicks)
{
	// так многопоточно, но не работает в WinXP: InterlockedExchangeAdd64(&_value, passedTimeInTicks);
	_value += passedTimeInTicks;
	return *this;
}

TimeAccum& TimeAccum::operator+=(const Timer& timer)
{
	// так многопоточно, но не работает в WinXP: InterlockedExchangeAdd64(&_value, timer.passed());
	_value += timer.passed();
	return *this;
}

void TimeAccum::addAndReset(Timer& timer)
{
	// так многопоточно, но не работает в WinXP: InterlockedExchangeAdd64(&_value, timer.stage());
	_value += timer.stage();
}

LONGLONG TimeAccum::value() const
{
	// 64-битное значение не обязательно возвращается за один такт шины,
	// поэтому может быть, что одна половина значения вернулась до того,
	// как её изменил другой поток, а вторая - после. Против этого заюзаем
	// атомарную функцию получения значения
	// так многопоточно, но не работает в WinXP: LONGLONG result = InterlockedExchangeAdd64(&_value, 0);
	LONGLONG result = _value;
	return result;
}

double TimeAccumEx::valueSec() const
{
	return _converter.toSec(value());
}

LONG TimeAccumEx::valueMsec() const
{
	return _converter.toMsec(value());
}

LONG TimeAccumEx::value100ns() const
{
	return _converter.to100ns(value());
}

TimeRange::TimeRange():
	start(0), end(0)
{}

LONGLONG TimeRange::length() const
{
	return end - start;
}

namespace details
{
	typedef LONG (NTAPI *NtQueryTimerResolution_Ptr)(
			PULONG minimumResolution,
			PULONG maximumResolution,
			PULONG currentResolution);
	__declspec(selectany)
	NtQueryTimerResolution_Ptr NtQueryTimerResolution = 0;
	__declspec(selectany)
	LONG NtQueryTimerResolution_init = 0;
	
	inline void SleepUpTo(LONGLONG endMoment, LONGLONG sleepTimeIn100ns)
	{
		// Кстати, для точного(?) отсчёта промежутка времени можно использовать
		// и CreateTimerQueueTimer (есть, начиная с XP)
		
		if( sleepTimeIn100ns > 10000 )
				// >10000 (>1 мс) потому что на практике прерывания времени не бывают
				// чаще, чем раз в 1мс; на современных машинах типично раз в 1-15мс,
				// на старых могло быть реже, скажем, 100мс; это по умолчанию, можно ускорить,
				// но чаще чем раз в 1мс на текущий момент вроде как не бывает
				// http://windowstimestamp.com/description
		{
			if( 0 == NtQueryTimerResolution_init )
			{
				// при многопоточном потоке переменная может инициализироваться дважды
				// (и библиотека загрузится тоже дважды) - в этом нет ровно никакой трагедии,
				// мы всё равно не освобождаем её до конца работы процесса
				NtQueryTimerResolution_Ptr func = 0;
				LONG init = 2;
				HMODULE mod = LoadLibrary(L"ntdll.dll");
				if( mod )
				{
					func = (NtQueryTimerResolution_Ptr)(
							GetProcAddress(mod, "NtQueryTimerResolution"));
					if( func )
						init = 1;
				}
				compile_assert(sizeof(NtQueryTimerResolution) == sizeof(LONG));
				InterlockedCompareExchange(
						reinterpret_cast<LONG*>(&NtQueryTimerResolution),
						reinterpret_cast<LONG&>(func),
						0); // тут мы полагаемся, что LONG(0) представляется теми же битами,
						    // что и нулевой указатель; на практике это так
				InterlockedCompareExchange(&NtQueryTimerResolution_init, init, 0);
			}
			
			if( 1 == NtQueryTimerResolution_init )
			{
				// resMin - это наибольший (несмотря на название) интервал между прерываниями
				// времени, resMax - наименьший возможный, а resActual - действующий сейчас.
				// Тонкость в том, что на resActual полагаться нельзя, он меняется постоянно.
				// Поэтому ориентируемся на самый грубый интервал, на resMin
				// Его типичное значение = 156000 = 15,6 мс
				ULONG resMin, resMax, resActual; // в 100нс интервалах
				(*NtQueryTimerResolution)(&resMin, &resMax, &resActual);
				
				// нет смысла погружаться в системный сон, если наш интервал меньше системного
				if( resMin > 0 && sleepTimeIn100ns > LONGLONG(resMin) )
				{
					// запросим спать на интервал меньше; если делать не так, легко можно
					// улететь в будущее
					LONGLONG toSleep = (sleepTimeIn100ns - resMin) / 10000;
					Sleep(toSleep > _I32_MAX ? DWORD(_I32_MAX-1) : DWORD(toSleep));

					/*LONG periodsToSleep = sleepTimeIn100ns / LONG(resMin);
					res.first = periodsToSleep;
					Sleep(periodsToSleep * resMin / 10000);*/
				}
			}
		}
		
		while(now() <= endMoment)
		{}
	}
} // namespace details

void SleepPrecTicks(LONGLONG sleepTimeInTicks)
{
	LONGLONG moment = now();
	if( sleepTimeInTicks <= 0 )
		return;
	details::SleepUpTo(
			moment + sleepTimeInTicks,
			TicksCvt().to100ns(sleepTimeInTicks));
}

void SleepPrecMs(LONG sleepTimeInMsec)
{
	LONGLONG moment = now();
	if( sleepTimeInMsec <= 0 )
		return;
	details::SleepUpTo(
			moment + TicksCvt().fromMsec(sleepTimeInMsec),
			sleepTimeInMsec * 10000);
}

void SleepPrecSec(double sleepTimeInSec)
{
	LONGLONG moment = now();
	if( sleepTimeInSec <= 0 )
		return;
	details::SleepUpTo(
			moment + TicksCvt().fromSec(sleepTimeInSec),
			LONG(sleepTimeInSec * 10000000));
}

} // namespace timers

#pragma pack(pop)
