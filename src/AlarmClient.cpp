#include "AlarmClient.hpp"

#include "Protocol/Auth.hpp"
#include "Protocol/Alarm.hpp"
#include "ProtocolSession.hpp"
#include "Config/ServerConfig.hpp"
#include "Config/ChannelConfig.hpp"

#include <Network/Client.hpp>
#include <Misc/Debug.hpp>

using namespace std::literals::chrono_literals;

#define AUTH_TIMEOUT    13s
#define RETRY_TIMEOUT   10s

using namespace boost::asio;
using namespace boost::system;

AlarmClient::AlarmClient(boost::asio::io_context& io, const ServerConfig *c)
    : CoroutineTask(io),
      m_config(c) {
    debug_print_this("");
}

AlarmClient::~AlarmClient() {
    debug_print_this("");
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
            debug_print_this(fmt("disabled channel %1% md on %2%") % (int)md[i] % server);
        } else {
            j++;
        }
    }

    if (j > 0) {
        onMotion(m_motionChannels, j);
    }
}

AlarmClient::TProtocolAwait AlarmClient::getProtocolSession() {
    TClient client(new Client(io()));
    client->setStrand(this);
    client->registerType<Session, ProtocolSession, Socket*>();
    registerStop(client);
    TSession s = co_await client->co_start(use_awaitable, m_config->getIp(), m_config->getPort());
    s->setStrand(this);
    co_return std::static_pointer_cast<ProtocolSession>(s);
}

AlarmClient::TAuthAwait AlarmClient::authorize() {
    auto session = co_await getProtocolSession();
    AuthProtocol* p = new AuthProtocol(m_config->getLogin(), m_config->getPass());
    session->setProtocol(p);
    co_await session->co_start(use_awaitable);
    co_return p->result();
}

TAwaitVoid AlarmClient::runAlarm(const AuthResult& auth) {
    auto session = co_await getProtocolSession();
    AlarmProtocol *p = new AlarmProtocol(auth);
    p->setMotionCallback([this](const AlarmProtocol::TMotion& md, const size_t& size) {
        motionHandler(md, size);
    });
    session->setProtocol(p);
    co_await session->co_start(use_awaitable);
    co_return;
}

TAwaitVoid AlarmClient::run() {
    while(running()) {
        try {
            auto authResult = co_await timeout(authorize(), AUTH_TIMEOUT);
            co_await runAlarm(authResult);
        } catch(const system_error& e) {
            if (e.code() == errc::operation_canceled) {
                throw;
            }
        } catch(...) { }

        co_await wait(RETRY_TIMEOUT);
    }
}
