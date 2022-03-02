#include "ServerConfig.hpp"
#include "ChannelConfig.hpp"

#include <boost/json.hpp>

ServerConfig::ServerConfig(const boost::json::object &o) :
    m_ip(get<std::string>(o, "ip")),
    m_port(get<std::int64_t>(o, "port")),
    m_login(get<std::string>(o, "login")),
    m_pass(get<std::string>(o, "pass")),
    m_comment(get<std::string, true>(o, "comment")) {
    registerType<ConfigItem, ChannelConfig, const boost::json::object&>();
}

void ServerConfig::init(const boost::json::object &o) {
    if (!o.contains("channels"))
        return;

    const auto& channels = o.at("channels");

    ArrayParser::TItems itms;
    parseArray(channels.as_array(), itms);

    bool enableAll = false;

    for (auto& c : itms) {
        ChannelConfig* cc = static_cast<ChannelConfig*>(c.get());

        if (cc->getComment() == "enable all")
            enableAll = true;

        if (cc->getId() < AlarmConstants::MOTION_MAX_CHANNUM_V30) {
            m_channels[cc->getId()].reset(cc);
            c.release();
        }
    }

    if (enableAll) {
        for (size_t i = 0 ; i < AlarmConstants::MOTION_MAX_CHANNUM_V30 ; i++) {
            if (m_channels[i].get() != nullptr)
                continue;

            const std::string input = "{\"id\":" + std::to_string(i) + "}";
            const auto obj = boost::json::parse(input).as_object();
            ChannelConfig* cc = static_cast<ChannelConfig*>(create<ConfigItem>(obj));
            m_channels[i].reset(cc);
        }
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
