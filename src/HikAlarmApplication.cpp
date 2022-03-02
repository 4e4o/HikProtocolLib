#include "HikAlarmApplication.h"
#include "AlarmClient.hpp"

#include "Config/ServerConfig.hpp"

#include <Config/Config.hpp>
#include <Misc/Debug.hpp>

#define PROG_NAME   "hikvision alarm application"

HikAlarmApplication::HikAlarmApplication(int argc, char* argv[])
    : BaseConfigApplication(PROG_NAME, argc, argv) {
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
        m_close.connect_extended([client](const boost::signals2::connection& con) {
            con.disconnect();
            client->stop();
        });
        client->start();
    }

    return true;
}
