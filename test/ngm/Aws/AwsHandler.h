#pragma once

#include <string>
#include <functional>

template<class... T>
class AwsHandler
{
public:
    typedef std::function<void(T...)> Handler;

private:
	Handler m_handler;

public:
	AwsHandler(Handler handler):
		m_handler(handler)
	{}
	AwsHandler() = delete;

    void invoke(T... args) {
        m_handler(args...);
	}
};

