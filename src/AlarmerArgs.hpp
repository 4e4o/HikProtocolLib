#ifndef ALARMER_ARGS_H
#define ALARMER_ARGS_H

#include <string>

struct AlarmerArgs {
    std::string ip;
    int port;
    std::string login;
    std::string pass;
};

#endif /* ALARMER_ARGS_H */
