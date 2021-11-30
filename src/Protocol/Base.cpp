#include "Base.hpp"
#include "ITransport.hpp"
#include "Debug.hpp"

#include <arpa/inet.h>

#include <string>
#include <string.h>

BaseProtocol::BaseProtocol()
    : m_transport(nullptr) {
    //    debug_print("BaseProtocol::BaseProtocol %p\n", this);
}

BaseProtocol::~BaseProtocol() {
    //    debug_print("BaseProtocol::~BaseProtocol %p\n", this);
}

bool BaseProtocol::start(ITransport* t) {
    m_transport = t;
    return true;
}

void BaseProtocol::sendCmd(const TData& d, TSendCallback c) {
    TData cmd(sizeof(TCmdSize) + d.size());
    TCmdSize* sizePtr = (TCmdSize*) cmd.data();
    *sizePtr = htonl(cmd.size());
    memcpy(cmd.data() + sizeof(TCmdSize), d.data(), d.size());
    m_transport->send(cmd, c);
}

void BaseProtocol::readCmd() {
    // read 4 byte cmd length
    m_transport->receive(sizeof(uint32_t), [this](uint8_t* data) -> bool {
        uint32_t size = ntohl(*((uint32_t*) data));

        if (size <= sizeof(size))
            return false;

        size -= sizeof(size);

        if (size < 4)
            return false;

        m_transport->receive(size, [this, size](uint8_t* data) -> bool {
            const uint32_t first = ntohl(*((uint32_t*) data));

            if (first == 0)
                return false;

            TCmdDesc desc{.firstU32 = first, .data{data, size}};
            return onCmd(desc);
        });

        return true;
    });
}

uint32_t BaseProtocol::readU32(const TCmdDesc& cmd, int offset) {
    return ntohl(*((uint32_t*) (cmd.data.data() + offset)));
}
