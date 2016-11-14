#pragma once

// � ����� ���������� �� ����������� ��� ������ � ���������� (�� �����������) ��������,
// ���������� ��� ������ ���������� � ������� ������� ��������. ��� ���������� ��������
// ������������� ������������� �� ����� ������ GetTickCount
// 
// ����� ����� ���������� � ������ ��������:
//   - ticks: LONGLONG, � ����� ��������
//   - msec:  LONG, ����� ������ ����������
//   - 100ns: LONG, 100-�������������� �����������, ������� ������� ������������ ������
//   - sec:   double, � ��������, � ��� ����� �������


// ��� ������ 64-������ Interlocked-������� ����, ����� ���������� ���� ���������
// �� ������� 64 ���, �.�. pragma pack �� ������ ���� ������ 8
#pragma pack(push, 8)

namespace timers
{

// ������ ��������������������� ��������
static const struct Uninitialized {} uninitialized;

// ��������� ���� �������� � ������ ������� ��������� � �������
// ����� ��������������� ����� �������������� ������������
class TicksCvt
{
public:
	// ��� ������������� ������� ������ ��������
	TicksCvt() throw();
	
	// ���� �������� -> �������
	double toSec(const LONGLONG& ticks) const throw();
	
	// ������� -> ���� ��������
	LONGLONG fromSec(double secs) const throw();

	// ���� �������� -> �����������
	LONG toMsec(const LONGLONG& ticks) const throw();
	
	// ����������� -> ���� ��������
	LONGLONG fromMsec(LONG msecs) const throw();

	// ���� �������� -> 100-������������� ��������� (� ��� �������� �������� �����)
	LONG to100ns(const LONGLONG& ticks) const throw();
	
	// 100-������������� ��������� -> ���� ��������
	LONGLONG from100ns(LONG timeIn100ns) const throw();

private:
	double _secsPersTicks; // ����� �������� � �������
};


// ���������� ������� ������ �������, � ����� ��������
LONGLONG now() throw();

// ����� ���������� ������� ������ ������� � ������ �������� ���������
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
	// ������� ������ � ����� ��������, ���������� ���������� ������� now()
	static LONGLONG ticks() throw();
	
	// � ������ �������� ���������
	LONG msec() const throw();
	LONG in100ns() const throw();
	double sec() const throw();

private:
	TicksCvt _converter;
};


// ���������� �������� ���������� ������� (� ������ �������� ���������)
// ����������� ������ ��������� ������� Sleep, �� ��� �������� �����
void SleepPrecTicks(LONGLONG sleepTimeInTicks) throw();
void SleepPrecMs(LONG sleepTimeInMsec) throw();
void SleepPrecSec(double sleepTimeInSec) throw();


// ����� ��� ��������� ����������� �������
// ����������, ������� ������� ������ � ������� ��������������� ������
// ��� � ���������� ������ reset
// ��������������� �� ��������������, �������� �� ������������� � ����� ������
class Timer
{
public:
	// ������ ������� ���������� � ������� ���������������
	Timer() throw();
	
	// ����������� �������, ����� ������ �� ���������������, ������� �����,
	// ����� ���� ����� ������ ������ reset
	explicit Timer(const Uninitialized&) throw();
	
	// ���������� ����� �� ��������
	
	// ������ ������ � ���������� �������
	void reset() throw();

	// ��������� ����� � ����� ��������
	LONGLONG passed() const throw();
	
	// ������� ��������� ����� (� �����) � �������� �������
	// �������, ����� ���� ������� �������� ���� �� ������ ������ ������ ��������
	// ����������, ��� ����������� passed � reset
	LONGLONG stage() throw();
	
private:
	LONGLONG _start;
};


// ������ �� ���������� �����������, ����� ����� ���������� ����� � ������
// �������� ���������
class TimerEx: public Timer
{
public:
	// ��������� ����� � ������ �������� ���������
	double passedSec() const throw();
	LONG passedMsec() const throw();
	LONG passed100ns() const throw();
	
private:
	TicksCvt _converter;
};


// ����������� �������
// �������� ��������� += ��������� � ���������� ������� ����� ����������
// -����� ��������������� ����� �������������� ������������-
// ��������������� �� ��������������, ��������������� �������
class TimeAccum
{
public:
	// ���������� ���������� ������� �������
	TimeAccum() throw();
	
	// �������� ������� � ����, ������ ������� �������� (� �����)
	LONGLONG clear() throw();
	
	// ���������� �����
	TimeAccum(const TimeAccum& from) throw();
	TimeAccum& operator=(const TimeAccum& from) throw();
	
	// �������� ����� ��������� ���������� ������� (� ����� ��������)
	TimeAccum& operator+=(const LONGLONG& passedTimeInTicks) throw();
	TimeAccum& operator+=(const Timer& timer) throw();
	
	// �������� ��������� �� ������� ����� � �������� (�������������) ������
	void addAndReset(Timer& timer) throw();
	
	// ������� �������� ������������ �������
	LONGLONG value() const throw();
	
private:
	mutable LONGLONG _value;
};


// ����������� ������� �� ���������� ���������������� ������� � ������ �������
// ���������
class TimeAccumEx: public TimeAccum
{
public:
	// ����������� ����� � �������� � ������������
	// (��� ������������� ������� �������� ������������, ������ ��� �������������
	// ������� ��������)
	double valueSec() const throw();
	LONG valueMsec() const throw();
	LONG value100ns() const throw();

private:
	TicksCvt _converter;
};


// �������� �������
// �� ���������� � ����� ��������
struct TimeRange
{
	// ����������� ��������� ������
	// (�� ������� ��������!)
	TimeRange() throw();
	
	LONGLONG start; // ������ ���������
	LONGLONG end;   // �����
	
	LONGLONG length() const throw(); // ����� ���������� ����������
};


} // namespace timers

#pragma pack(pop)
