#include "Config.hpp"
#include "Misc/Debug.hpp"
#include "ServerConfig.hpp"
#include "ChannelConfig.hpp"

#include <iostream>
#include <fstream>

#include <boost/json/src.hpp>

Config::Config() {
     registerDefaultType<ServerConfig, Config*, const boost::json::object&>();
     registerDefaultType<ChannelConfig, const boost::json::object&>();
}

Config::~Config() {
}

bool Config::readConfig(TServers& result, const std::string& config_file) {
    try {
        std::ifstream ifs(config_file);
        std::string input(std::istreambuf_iterator<char>(ifs), {});

        auto parsed_data = boost::json::parse(input);
        const auto& servers = parsed_data.at("servers");

        for (const auto& server : servers.as_array()) {
            const boost::json::object& obj = server.as_object();

            if (!ConfigItem::isEnabled(obj))
                continue;

            ServerConfig *sc = create<ServerConfig>(this, obj);

            if (sc == nullptr)
                continue;

            result.emplace_back(sc);
        }
    } catch (std::exception const& e) {
        std::cout << "Config parse error: " << e.what() << std::endl;
        return false;
    }

    return true;
}
