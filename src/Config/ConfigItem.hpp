#ifndef CONFIG_ITEM_HPP
#define CONFIG_ITEM_HPP

#include <string>

namespace boost::json {
class string;
class object;
}

class ConfigItem {
public:
    virtual ~ConfigItem();

    static std::string toStdString(const boost::json::string& s1);
    static bool isEnabled(const boost::json::object&);
};

#endif /* CONFIG_ITEM_HPP */
