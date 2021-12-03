#include "Timer.hpp"
#include "Debug.hpp"

Timer::Timer(boost::asio::io_context& io_context, int sec)
    : m_sec(sec),
      m_ioContext(io_context),
      m_timeout(io_context) {
    debug_print("Timer::Timer %p\n", this);
}

Timer::~Timer() {
    debug_print("Timer::~Timer %p\n", this);
}

boost::asio::io_context& Timer::ioContext() {
    return m_ioContext;
}

void Timer::startTimer() {
//    debug_print("Timer::startTimer %p %i\n", this, m_sec);

    m_timeout.expires_from_now(boost::posix_time::seconds(m_sec));
    m_timeout.async_wait([this](const boost::system::error_code& error) {
        if (error)
            return;

        onTimeout();
    });
}

void Timer::stopTimer() {
    m_timeout.cancel();
}
