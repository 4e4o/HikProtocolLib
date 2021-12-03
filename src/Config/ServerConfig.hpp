#ifndef SERVER_CONFIG_HPP
#define SERVER_CONFIG_HPP

#include "ConfigItem.hpp"
#include "Protocol/AlarmConstants.hpp"

#include <memory>

class Config;
class ChannelConfig;

class ServerConfig : public ConfigItem {
public:
    typedef std::unique_ptr<ChannelConfig> TChannels[AlarmConstants::MOTION_MAX_CHANNUM_V30];

    ServerConfig(Config*, const boost::json::object&);

    const std::string& getIp() const;
    int getPort() const;
    const std::string& getLogin() const;
    const std::string& getPass() const;
    const TChannels& channels() const;
    const std::string& getComment() const;

private:
    std::string m_ip;
    int m_port;
    std::string m_login;
    std::string m_pass;
    std::string m_comment;
    TChannels m_channels;
};

#endif /* SERVER_CONFIG_HPP */
