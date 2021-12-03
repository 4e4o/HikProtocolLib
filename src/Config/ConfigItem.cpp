#include "ConfigItem.hpp"

#include <boost/json.hpp>

ConfigItem::~ConfigItem() {
}

std::string ConfigItem::toStdString(const boost::json::string& s1) {
    return std::string(s1.begin(), s1.end());
}

bool ConfigItem::isEnabled(const boost::json::object& o) {
    if (o.contains("enabled"))
        return o.at("enabled").as_bool();

    return true;
}
