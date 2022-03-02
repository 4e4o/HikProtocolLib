#include "AlarmClient.hpp"

#include "Protocol/Auth.hpp"
#include "Protocol/Alarm.hpp"
#include "ProtocolSession.hpp"
#include "Config/ServerConfig.hpp"
#include "Config/ChannelConfig.hpp"

#include <Network/ClientManager.hpp>
#include <Misc/Debug.hpp>

#define RECONNECT_SEC           10
#define RECV_TIMEOUT_SEC        20
#define AUTH_RECV_TIMEOUT_SEC   10

using boost::signals2::connection;

AlarmClient::AlarmClient(boost::asio::io_context& io, const ServerConfig *c)
    : Timer(io, RECONNECT_SEC),
      m_stopped(false),
      m_config(c) {
    onTimeout.connect([this] (Timer*) {
        debug_print(boost::format("AlarmClient::onTimeout %1%") % this);
        start();
    });

    debug_print(boost::format("AlarmClient::AlarmClient %1%") % this);
}

AlarmClient::~AlarmClient() {
    debug_print(boost::format("AlarmClient::~AlarmClient %1%") % this);
}

const ServerConfig* AlarmClient::config() const {
    return m_config.get();
}

void AlarmClient::motionHandler(const AlarmProtocol::TMotion& md, const size_t& size) {
    STRAND_ASSERT(this);
    const ServerConfig::TChannels& channels = m_config->channels();
    size_t j = 0;

    for (size_t i = 0 ; i < size ; i++) {
        m_motionChannels[j] = channels[md[i]].get();

        if (m_motionChannels[j] == nullptr) {
            const std::string server = config()->getComment().empty() ?
                        config()->getIp() : config()->getComment();
            debug_print(boost::format("disabled channel %1% md on %2% ") % (int)md[i] % server);
        } else {
            j++;
        }
    }

    if (j > 0) {
        onMotion(m_motionChannels, j);
    }
}

std::shared_ptr<ClientManager> AlarmClient::createClient() {
    std::shared_ptr<ClientManager> client(new ClientManager(io()));
    client->setStrand(this, false);
    client->registerType<Session, ProtocolSession, boost::asio::io_context&>();
    m_stopClients.connect(decltype(m_stopClients)::slot_type(
                              &ClientManager::stop, client.get()).track_foreign(client));
    return client;
}

void AlarmClient::start() {
    auto self = shared_from_this();
    post([self] {
        self->startImpl();
    });
}

void AlarmClient::stop() {
    auto self = shared_from_this();
    post([self] {
        if (self->m_stopped)
            return;

        self->m_stopped = true;
        self->m_stopClients();
    });
}

void AlarmClient::startImpl() {
    auto self = shared_from_this();
    auto client = createClient();
    client->onNewSession.connect([self](Session *s) {
        s->setReceiveTimeout(AUTH_RECV_TIMEOUT_SEC);
        ProtocolSession *session = static_cast<ProtocolSession*>(s);
        AuthProtocol* p = new AuthProtocol(self->m_config->getLogin(), self->m_config->getPass());
        session->setProtocol(p);
        session->onProtocolDone.connect([self, p]() {
            if (p->loggedIn()) {
                self->runAlarm(p->result());
            } else {
                self->restart();
            }
        });
    });

    client->start(m_config->getIp(), m_config->getPort());
}

void AlarmClient::runAlarm(const AuthResult& auth) {
    auto self = shared_from_this();
    post([self, auth] {
        self->runAlarmImpl(auth);
    });
}

void AlarmClient::runAlarmImpl(const AuthResult& auth) {
    auto self = shared_from_this();
    auto client = createClient();
    client->onNewSession.connect([self, auth](Session *s) {
        s->setReceiveTimeout(RECV_TIMEOUT_SEC);
        ProtocolSession *session = static_cast<ProtocolSession*>(s);
        AlarmProtocol *p = new AlarmProtocol(auth);
        p->setMotionCallback([self](const AlarmProtocol::TMotion& md, const size_t& size) {
            self->motionHandler(md, size);
        });
        session->setProtocol(p);
        session->onDestroy.connect([self]() {
            self->restart();
        });
    });

    client->start(m_config->getIp(), m_config->getPort());
}

void AlarmClient::restart() {
    auto self = shared_from_this();
    post([self] {
        if (self->m_stopped)
            return;

        self->startTimer();
    });
}
