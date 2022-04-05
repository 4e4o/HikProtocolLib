#ifndef ITRANSPORT_HPP
#define ITRANSPORT_HPP

#include <vector>

#include <Coroutine/Awaitables.hpp>

typedef boost::asio::awaitable<const uint8_t*> TAwaitData;

class ITransport {
public:
    virtual ~ITransport() { }

    typedef std::vector<uint8_t> TData;

    virtual TAwaitSize send(const TData& data) = 0;
    virtual TAwaitData receive(size_t) = 0;
};

#endif /* ITRANSPORT_HPP */
