#include "HikAlarmApplication.h"
#include "AlarmClient.hpp"

#include "Config/ServerConfig.hpp"

#include <Misc/Lifecycle.hpp>
#include <Config/Config.hpp>
#include <Misc/Debug.hpp>

HikAlarmApplication::HikAlarmApplication(int argc, char* argv[])
    : BaseConfigApplication(argc, argv) {
    config()->registerType<ConfigItem, ServerConfig, const boost::json::object&>();
}

HikAlarmApplication::~HikAlarmApplication() {
}

void HikAlarmApplication::doExit() {
    m_close();
}

void HikAlarmApplication::onNewClient(AlarmClient*) {
}

bool HikAlarmApplication::start(TConfigItems &hik_servers) {
    for (auto& config : hik_servers) {
        ServerConfig * sc = static_cast<ServerConfig*>(config.release());
        std::shared_ptr<AlarmClient> client(new AlarmClient(io(), sc));
        onNewClient(client.get());
        Lifecycle::connectTrack(m_close, client, &AlarmClient::stop);
        client->start();
    }

    return true;
}
