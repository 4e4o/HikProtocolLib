#ifndef SERVER_CONFIG_HPP
#define SERVER_CONFIG_HPP

#include <Config/ConfigItem.hpp>
#include <Config/ArrayParser.hpp>

#include "Protocol/AlarmConstants.hpp"

#include <memory>

class Config;
class ChannelConfig;

class ServerConfig : public ConfigItem, public ArrayParser {
public:
    typedef std::unique_ptr<ChannelConfig> TChannels[AlarmConstants::MOTION_MAX_CHANNUM_V30];

    ServerConfig(const boost::json::object&);

    const std::string& getIp() const;
    int getPort() const;
    const std::string& getLogin() const;
    const std::string& getPass() const;
    const TChannels& channels() const;
    const std::string& getComment() const;

private:
    void init(const boost::json::object&) override final;

    const std::string m_ip;
    const int m_port;
    const std::string m_login;
    const std::string m_pass;
    const std::string m_comment;
    TChannels m_channels;
};

#endif /* SERVER_CONFIG_HPP */
