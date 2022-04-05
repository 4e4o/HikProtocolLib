#include "Base.hpp"

#include <Misc/Debug.hpp>

#include <arpa/inet.h>

#include <string>

#define MINIMAL_CMD_LENGTH 4

BaseProtocol::BaseProtocol()
    : m_transport(nullptr) {
    //debug_print_this("");
}

BaseProtocol::~BaseProtocol() {
    //debug_print_this("");
}

TAwaitVoid BaseProtocol::start(ITransport* t) {
    m_transport = t;
    co_return;
}

TAwaitSize BaseProtocol::sendCmd(const ITransport::TData& d) {
    TData cmd(sizeof(TCmdSize) + d.size());
    TCmdSize* sizePtr = (TCmdSize*) cmd.data();
    *sizePtr = htonl(cmd.size());
    memcpy(cmd.data() + sizeof(TCmdSize), d.data(), d.size());
    return m_transport->send(cmd);
}

BaseProtocol::TAwaitCmd BaseProtocol::readCmd() {
    // read 4 byte cmd length
    const uint8_t* cmdLength = co_await m_transport->receive(4);
    uint32_t size = ntohl(*((uint32_t*) cmdLength));

    // size must be > (sizeof(size) + body_length)
    if (size <= sizeof(size))
        throw std::runtime_error("invalid cmd length");

    size -= sizeof(size);

    if (size < MINIMAL_CMD_LENGTH)
        throw std::runtime_error("invalid cmd length");

    const uint8_t* body = co_await m_transport->receive(size);
    const uint32_t first = ntohl(*((uint32_t*) body));

    // i think it is error indicator
    if (first == 0)
        throw std::runtime_error("cmd body first == 0");

    co_return TCmdDesc{.firstU32 = first, .data{body, size}};
}

uint32_t BaseProtocol::readU32(const TCmdDesc& cmd, int offset) {
    return ntohl(*((uint32_t*) (cmd.data.data() + offset)));
}
