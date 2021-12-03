#ifndef TIMER_HPP
#define TIMER_HPP

#include <boost/asio/deadline_timer.hpp>

class Timer {
public:
    Timer(boost::asio::io_context& io_context, int);
    virtual ~Timer();

protected:
    boost::asio::io_context& ioContext();

    void startTimer();
    void stopTimer();

    virtual void onTimeout() = 0;

private:
    int m_sec;
    boost::asio::io_context& m_ioContext;
    boost::asio::deadline_timer m_timeout;
};

#endif /* TIMER_HPP */
