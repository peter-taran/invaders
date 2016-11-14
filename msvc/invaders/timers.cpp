#include "stdafx.h"

#pragma pack(push, 8)

namespace timers
{
    
// [19.10.2012 tarkil] ���������� �������� � ���, ��� �� ����������������� �������
// ����������� ������ QueryPerformanceCounter ����� ���� ������ ��������, ���
// ����������, ��� ������ ��-�� ���� BIOS ��� HAL.
// ��. http://msdn.microsoft.com/en-us/library/ms644904(VS.85).aspx?ppud=4
// � ��� ��������� �� \\zenith-co, ���������� ��� VirtualBox
// ������� ��������� ������������ �������� ����������� � ������ ����� �������� �������
// �����������, ����� ��� �� ���� ������ � ���������� ���������.
// ��, �������� � �������� � ��������, �� �� ������� ���� ����� �� ��� �����
// �������������� ������, ���� ����� ���� ����� ���������
// TODO: � ����������� ������������ ������ ������������ __declspec(thread), ���� ��� ����,
// �� �������� ����� �� ��� ������ � �� ����� ������� � ����� ������� ����� ��������
// ������; ���� ������ ��� ���������, �������� ������� (��� ��� - ���������)
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
			// �������� �� 100000, ����� ��������� ����� ���� ������ ������
			// ��������� ���� ��� ���������� ������� ��������, �� - ���������
	}
	return result.QuadPart;
}

TicksCvt::TicksCvt()
{
	LARGE_INTEGER ratio;
	if( QueryPerformanceFrequency(&ratio) )
		_secsPersTicks = 1.0 / ratio.QuadPart;
	else
		_secsPersTicks = 0.00000001; // ��� GetTickCount: 1000 * 100000 �����/���
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
		// ��� ������������, �� �� �������� � WinXP: InterlockedExchange64(&_value, from._value);
		_value = from._value;
	}
	return *this;
}

TimeAccum& TimeAccum::operator+=(const LONGLONG& passedTimeInTicks)
{
	// ��� ������������, �� �� �������� � WinXP: InterlockedExchangeAdd64(&_value, passedTimeInTicks);
	_value += passedTimeInTicks;
	return *this;
}

TimeAccum& TimeAccum::operator+=(const Timer& timer)
{
	// ��� ������������, �� �� �������� � WinXP: InterlockedExchangeAdd64(&_value, timer.passed());
	_value += timer.passed();
	return *this;
}

void TimeAccum::addAndReset(Timer& timer)
{
	// ��� ������������, �� �� �������� � WinXP: InterlockedExchangeAdd64(&_value, timer.stage());
	_value += timer.stage();
}

LONGLONG TimeAccum::value() const
{
	// 64-������ �������� �� ����������� ������������ �� ���� ���� ����,
	// ������� ����� ����, ��� ���� �������� �������� ��������� �� ����,
	// ��� � ������� ������ �����, � ������ - �����. ������ ����� �������
	// ��������� ������� ��������� ��������
	// ��� ������������, �� �� �������� � WinXP: LONGLONG result = InterlockedExchangeAdd64(&_value, 0);
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
		// ������, ��� �������(?) ������� ���������� ������� ����� ������������
		// � CreateTimerQueueTimer (����, ������� � XP)
		
		if( sleepTimeIn100ns > 10000 )
				// >10000 (>1 ��) ������ ��� �� �������� ���������� ������� �� ������
				// ����, ��� ��� � 1��; �� ����������� ������� ������� ��� � 1-15��,
				// �� ������ ����� ���� ����, ������, 100��; ��� �� ���������, ����� ��������,
				// �� ���� ��� ��� � 1�� �� ������� ������ ����� ��� �� ������
				// http://windowstimestamp.com/description
		{
			if( 0 == NtQueryTimerResolution_init )
			{
				// ��� ������������� ������ ���������� ����� ������������������ ������
				// (� ���������� ���������� ���� ������) - � ���� ��� ����� ������� ��������,
				// �� �� ����� �� ����������� � �� ����� ������ ��������
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
						0); // ��� �� ����������, ��� LONG(0) �������������� ���� �� ������,
						    // ��� � ������� ���������; �� �������� ��� ���
				InterlockedCompareExchange(&NtQueryTimerResolution_init, init, 0);
			}
			
			if( 1 == NtQueryTimerResolution_init )
			{
				// resMin - ��� ���������� (�������� �� ��������) �������� ����� ������������
				// �������, resMax - ���������� ���������, � resActual - ����������� ������.
				// �������� � ���, ��� �� resActual ���������� ������, �� �������� ���������.
				// ������� ������������� �� ����� ������ ��������, �� resMin
				// ��� �������� �������� = 156000 = 15,6 ��
				ULONG resMin, resMax, resActual; // � 100�� ����������
				(*NtQueryTimerResolution)(&resMin, &resMax, &resActual);
				
				// ��� ������ ����������� � ��������� ���, ���� ��� �������� ������ ����������
				if( resMin > 0 && sleepTimeIn100ns > LONGLONG(resMin) )
				{
					// �������� ����� �� �������� ������; ���� ������ �� ���, ����� �����
					// ������� � �������
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
