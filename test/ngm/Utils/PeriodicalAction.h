#pragma once

#include <chrono>
#include <algorithm>
#include <thread>

class PeriodicalAction
{
public:
	PeriodicalAction(std::function<void()> func, size_t duration_ms);
	PeriodicalAction() = delete;
	PeriodicalAction(const PeriodicalAction &) = delete;
	PeriodicalAction(PeriodicalAction &&) = delete;
    void yield();
	void set_duration(size_t duration_ms);
	size_t duration();

private:
	std::function<void()> m_func;
	std::chrono::milliseconds m_duration;
	std::chrono::system_clock::time_point m_last;
};
