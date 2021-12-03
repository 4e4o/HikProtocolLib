#include "AlarmClient.hpp"

#include "Protocol/Auth.hpp"
#include "Protocol/Alarm.hpp"
#include "Config/ServerConfig.hpp"
#include "Config/ChannelConfig.hpp"
#include "Misc/Debug.hpp"
#include "Connection.hpp"

#define RETRY_SEC   5

AlarmClient::AlarmClient(boost::asio::io_context& io, const ServerConfig *c)
    : Timer(io, RETRY_SEC), m_config(c) {
}

const ServerConfig* AlarmClient::config() const {
    return m_config.get();
}

void AlarmClient::runAlarm(const AuthResult& auth) {
    boost::shared_ptr<Connection> client(new Connection(ioContext()));
    AlarmProtocol *p = new AlarmProtocol(auth);

    p->setMotionCallback([this](const AlarmProtocol::TMotion& md, const size_t& size) {
        if (!m_motionCallback)
            return;

        const ServerConfig::TChannels& channels = m_config->channels();

        for (size_t i = 0 ; i < size ; i++) {
            m_motionChannels[i] = channels[md[i]].get();
        }

        m_motionCallback(m_motionChannels, size);
    });

    client->setOnDestroy([this]() {
        startTimer();
    });

    client->start(p, m_config->getIp(), m_config->getPort());
}

void AlarmClient::start(const TMotionCallback& c) {
    m_motionCallback = c;

    boost::shared_ptr<Connection> client(new Connection(ioContext()));
    AuthProtocol* p = new AuthProtocol(m_config->getLogin(), m_config->getPass());

    client->setOnDestroy([p, this]() {
        if (p->loggedIn()) {
            runAlarm(p->result());
        } else {
            startTimer();
        }
    });

    client->start(p, m_config->getIp(), m_config->getPort());
}

void AlarmClient::onTimeout() {
    debug_print("AlarmClient::onTimeout, restarting %p\n", this);
    start(m_motionCallback);
}
