#ifndef ALARM_CLIENT_HPP
#define ALARM_CLIENT_HPP

#include "Protocol/Alarm.hpp"

#include <boost/asio/io_context.hpp>
#include <boost/enable_shared_from_this.hpp>

#include <Misc/Timer.hpp>

class ServerConfig;
class ChannelConfig;
class ClientManager;

class AlarmClient : public std::enable_shared_from_this<AlarmClient>, public Timer {
public:
    typedef ChannelConfig* TMotion[AlarmConstants::MOTION_MAX_CHANNUM_V30];

    AlarmClient(boost::asio::io_context&, const ServerConfig*);
    ~AlarmClient();

    void start();
    void stop();

    const ServerConfig* config() const;

    boost::signals2::signal<void(const TMotion&, const size_t&)> onMotion;

private:
    void runAlarm(const AuthResult& auth);
    void motionHandler(const AlarmProtocol::TMotion& md, const size_t& size);
    void restart();
    std::shared_ptr<ClientManager> createClient();
    void startImpl();
    void runAlarmImpl(const AuthResult& auth);

    boost::signals2::signal<void()> m_stopClients;
    bool m_stopped;
    std::unique_ptr<const ServerConfig> m_config;
    ChannelConfig* m_motionChannels[AlarmConstants::MOTION_MAX_CHANNUM_V30];
};

#endif /* ALARM_CLIENT_HPP */
