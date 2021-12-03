#include "ServerConfig.hpp"
#include "Config.hpp"
#include "ChannelConfig.hpp"

#include <boost/json.hpp>

ServerConfig::ServerConfig(Config* config, const boost::json::object &o) {
    m_ip = toStdString(o.at("ip").as_string());
    m_port = o.at("port").as_int64();
    m_login = toStdString(o.at("login").as_string());
    m_pass = toStdString(o.at("pass").as_string());

    if (o.contains("comment"))
        m_comment = toStdString(o.at("comment").as_string());

    for (size_t i = 0 ; i < AlarmConstants::MOTION_MAX_CHANNUM_V30 ; i++) {
        const std::string input = "{\"id\":" + std::to_string(i) + "}";
        const auto obj = boost::json::parse(input).as_object();
        m_channels[i].reset(config->create<ChannelConfig>(obj));
    }

    if (!o.contains("channels"))
        return;

    const auto& channels = o.at("channels");

    for (const auto& channel : channels.as_array()) {
        const boost::json::object& obj = channel.as_object();

        if (!ConfigItem::isEnabled(obj))
            continue;

        std::unique_ptr<ChannelConfig> channel_desc(config->create<ChannelConfig>(obj));

        if (channel_desc->getId() < AlarmConstants::MOTION_MAX_CHANNUM_V30)
            m_channels[channel_desc->getId()].reset(channel_desc.release());
    }
}

const std::string& ServerConfig::getIp() const {
    return m_ip;
}

int ServerConfig::getPort() const {
    return m_port;
}

const std::string& ServerConfig::getLogin() const {
    return m_login;
}

const std::string& ServerConfig::getPass() const {
    return m_pass;
}

const ServerConfig::TChannels& ServerConfig::channels() const {
    return m_channels;
}

const std::string& ServerConfig::getComment() const {
    return m_comment;
}
