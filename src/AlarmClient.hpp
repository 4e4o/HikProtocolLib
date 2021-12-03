#ifndef ALARM_CLIENT_HPP
#define ALARM_CLIENT_HPP

#include "Protocol/Alarm.hpp"
#include "Misc/Timer.hpp"

class ServerConfig;
class ChannelConfig;

class AlarmClient : public Timer {
public:
    typedef ChannelConfig* TMotion[AlarmConstants::MOTION_MAX_CHANNUM_V30];
    typedef std::function<void(const TMotion&, const size_t&)> TMotionCallback;

    AlarmClient(boost::asio::io_context&, const ServerConfig*);

    void start(const TMotionCallback&);

    const ServerConfig* config() const;

private:
    void onTimeout() override final;

    void runAlarm(const AuthResult& auth);

    std::unique_ptr<const ServerConfig> m_config;
    TMotionCallback m_motionCallback;
    ChannelConfig* m_motionChannels[AlarmConstants::MOTION_MAX_CHANNUM_V30];
};

#endif /* ALARM_CLIENT_HPP */
