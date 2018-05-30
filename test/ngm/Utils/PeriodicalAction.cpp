#include "PeriodicalAction.h"

PeriodicalAction::PeriodicalAction(std::function<void()> func,
		size_t duration_ms):
		m_func(func),
		m_duration(duration_ms),
		m_last(std::chrono::system_clock::now() - m_duration)
{
}

void PeriodicalAction::yield()
{
	auto now = std::chrono::system_clock::now();
	if (now - m_last >= m_duration) {
		m_func();
        m_last += m_duration;
        if(now - m_last > m_duration)
            m_last = now;
	}
}

void PeriodicalAction::set_duration(size_t duration_ms)
{
	m_duration = std::chrono::milliseconds(duration_ms);
}

size_t PeriodicalAction::duration()
{
	return m_duration.count();
}
