#ifndef BASE_PROTOCOL_HPP
#define BASE_PROTOCOL_HPP

#include "ITransport.hpp"

#include <span>

class BaseProtocol {
public:
    typedef ITransport::TData TData;

    BaseProtocol();
    virtual ~BaseProtocol();

    virtual TAwaitVoid start(ITransport*);

    struct TCmdDesc {
        uint32_t firstU32;
        std::span<const uint8_t> data;
    };

    typedef boost::asio::awaitable<TCmdDesc> TAwaitCmd;

protected:
    TAwaitSize sendCmd(const TData&);
    TAwaitCmd readCmd();

    static uint32_t readU32(const TCmdDesc&, int offset);

private:
    typedef uint32_t TCmdSize;

    ITransport* m_transport;
};

#endif /* BASE_PROTOCOL_HPP */
