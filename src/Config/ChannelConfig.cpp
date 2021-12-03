#include "ChannelConfig.hpp"

#include <boost/json.hpp>

ChannelConfig::ChannelConfig(const boost::json::object &o) {
    m_id = o.at("id").as_int64();

    if (o.contains("comment"))
        m_comment = toStdString(o.at("comment").as_string());
}

uint8_t ChannelConfig::getId() const {
    return m_id;
}

const std::string& ChannelConfig::getComment() const {
    return m_comment;
}
