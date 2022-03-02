#ifndef ITRANSPORT_HPP
#define ITRANSPORT_HPP

#include <vector>
#include <functional>

#include <cstdint>

class ITransport {
public:
    typedef std::vector<uint8_t> TData;
    typedef std::function<void()> TSendCallback;
    typedef std::function<bool(const uint8_t*)> TReceiveCallback;

    virtual ~ITransport() { }

    virtual void send(const TData& data, TSendCallback c) = 0;
    virtual void receive(size_t, TReceiveCallback c) = 0;
};

#endif /* ITRANSPORT_HPP */
