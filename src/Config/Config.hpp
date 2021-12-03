#ifndef CONFIG_HPP
#define CONFIG_HPP

#include "Misc/GenericFactory.hpp"
#include "ConfigItem.hpp"

#include <list>
#include <memory>

class ServerConfig;

class Config : public GenericFactory<ConfigItem> {
public:
    typedef std::list<std::unique_ptr<ServerConfig>> TServers;

    Config();
    virtual ~Config();

protected:
    bool readConfig(TServers&, const std::string&);
};

#endif /* CONFIG_HPP */
