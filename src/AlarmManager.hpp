#ifndef ALARM_MANAGER_HPP
#define ALARM_MANAGER_HPP

#include <boost/asio/io_context.hpp>

#include <list>

#include "Config/Config.hpp"
#include "Config/ServerConfig.hpp"
#include "Config/ChannelConfig.hpp"
#include "AlarmClient.hpp"

class AlarmManager : public Config {
public:
    using TMotion = AlarmClient::TMotion;
    typedef std::function<void(AlarmClient*, const TMotion&, const size_t&)> TMotionCallback;

    AlarmManager(int argc, char **argv);
    ~AlarmManager();

    int run();

    void setMotionCallback(TMotionCallback);

private:
    void startClients();

    std::string m_configPath;
    boost::asio::io_context m_io;
    std::list<std::unique_ptr<AlarmClient>> m_clients;
    TMotionCallback m_motionCallback;
};

#endif /* ALARM_MANAGER_HPP */
