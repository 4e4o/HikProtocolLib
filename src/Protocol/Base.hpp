#ifndef BASE_PROTOCOL_HPP
#define BASE_PROTOCOL_HPP

#include <vector>
#include <string>
#include <functional>
#include <span>

class ITransport;

class BaseProtocol {
public:
    typedef std::vector<uint8_t> TData;
    typedef std::function<void()> TSendCallback;
    typedef std::function<bool(uint8_t*)> TReceiveCallback;

    BaseProtocol();
    virtual ~BaseProtocol();

    virtual bool start(ITransport*);

    struct TCmdDesc {
        uint32_t firstU32;
        std::span<const uint8_t> data;
    };

protected:
    void sendCmd(const TData& d, TSendCallback c);
    void readCmd();

    virtual bool onCmd(const TCmdDesc&) = 0;

    static uint32_t readU32(const TCmdDesc&, int offset);

private:
    typedef uint32_t TCmdSize;

    ITransport* m_transport;
};

#endif /* BASE_PROTOCOL_HPP */
