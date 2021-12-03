#include "AlarmManager.hpp"
#include "AlarmClient.hpp"

AlarmManager::AlarmManager(int argc, char **argv) {
    if (argc > 1)
        m_configPath = argv[1];
}

AlarmManager::~AlarmManager() {
}

void AlarmManager::setMotionCallback(TMotionCallback c) {
    m_motionCallback = c;
}

void AlarmManager::startClients() {
    for (auto &c : m_clients) {
        c->start([this, &c](const TMotion& md, const size_t& size) {
            if (!m_motionCallback)
                return;

            m_motionCallback(c.get(), md, size);
        });
    }
}

int AlarmManager::run() {
    {
        TServers servers;

        if (!readConfig(servers, m_configPath))
            return 1;

        for (auto &s : servers) {
            m_clients.emplace_back(new AlarmClient(m_io, s.release()));
        }
    }

    startClients();
    return m_io.run();
}
