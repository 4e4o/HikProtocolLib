#ifndef CHANNEL_CONFIG_HPP
#define CHANNEL_CONFIG_HPP

#include "ConfigItem.hpp"

#include <string>

class ChannelConfig : public ConfigItem {
public:
    ChannelConfig(const boost::json::object&);

    uint8_t getId() const;
    const std::string& getComment() const;

private:
    uint8_t m_id;
    std::string m_comment;
};

#endif /* CHANNEL_CONFIG_HPP */
