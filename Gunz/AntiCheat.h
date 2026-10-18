#pragma once

// Exit Application
__forceinline void ExitAPP() {
	ExitProcess(NULL);
}

// AntiCheat
class AntiCheat
{
private:
	HINSTANCE		m_hInstance;

public:
	AntiCheat();
	~AntiCheat();

	static AntiCheat* GetInstance();

	bool OnLauncherInit();
	static void Main();

	void			SetMainWnd(HINSTANCE hInstance);
	HINSTANCE		GetMainWnd();
};

inline AntiCheat* ZGetAntiCheat() { return AntiCheat::GetInstance(); }

#define SCAN_START GetModuleHandle(NULL)
#define SCAN_END GetModuleHandle(0)+0x4CFFFF
#define CODE_SIZE SCAN_END-SCAN_START

#define IS_VALID_HANDLE(handle)				(handle&& handle != INVALID_HANDLE_VALUE)

#include <chrono>

template <typename T>
class CTimer
{
public:
	CTimer()
	{
		m_tStartPoint = std::chrono::high_resolution_clock::now();
	}
	~CTimer() = default;

	auto diff()
	{
		return std::chrono::duration_cast<T>(std::chrono::high_resolution_clock::now() - m_tStartPoint).count();
	}

	void reset()
	{
		m_tStartPoint = std::chrono::high_resolution_clock::now();
	}

private:
	std::chrono::time_point <std::chrono::high_resolution_clock> m_tStartPoint;
};
