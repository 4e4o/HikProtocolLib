#include "ChannelConfig.hpp"

#include <boost/json.hpp>

ChannelConfig::ChannelConfig(const boost::json::object &o)
    : m_id(get<std::int64_t>(o, "id")),
      m_comment(get<std::string, true>(o, "comment")) {
}

uint8_t ChannelConfig::getId() const {
    return m_id;
}

const std::string& ChannelConfig::getComment() const {
    return m_comment;
}
