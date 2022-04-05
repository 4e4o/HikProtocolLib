#ifndef ALARM_CLIENT_HPP
#define ALARM_CLIENT_HPP

#include "Protocol/Alarm.hpp"

#include <Coroutine/CoroutineTask.hpp>

class ServerConfig;
class ChannelConfig;
class ProtocolSession;

class AlarmClient : public CoroutineTask<void> {
public:
    typedef ChannelConfig* TMotion[AlarmConstants::MOTION_MAX_CHANNUM_V30];

    AlarmClient(boost::asio::io_context&, const ServerConfig*);
    ~AlarmClient();

    const ServerConfig* config() const;

    boost::signals2::signal<void(const TMotion&, const size_t&)> onMotion;

private:
    typedef std::shared_ptr<ProtocolSession> TProtocolSession;
    typedef boost::asio::awaitable<TProtocolSession> TProtocolAwait;
    typedef boost::asio::awaitable<AuthResult> TAuthAwait;

    TAwaitVoid run() override final;

    void motionHandler(const AlarmProtocol::TMotion& md, const size_t& size);
    TProtocolAwait getProtocolSession();
    TAuthAwait authorize();
    TAwaitVoid runAlarm(const AuthResult& auth);

    std::unique_ptr<const ServerConfig> m_config;
    ChannelConfig* m_motionChannels[AlarmConstants::MOTION_MAX_CHANNUM_V30];
};

#endif /* ALARM_CLIENT_HPP */
