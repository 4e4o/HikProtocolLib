#include "ProtocolSession.hpp"
#include "Protocol/Base.hpp"

#include <Network/Session/Operations/SessionReader.hpp>
#include <Network/Session/Operations/SessionWriter.hpp>

using namespace std::literals::chrono_literals;

#define RECV_TIMEOUT    20s

ProtocolSession::ProtocolSession(Socket* s)
    : Session(s) {
}

ProtocolSession::~ProtocolSession() {
}

void ProtocolSession::setProtocol(BaseProtocol* p) {
    m_protocol.reset(p);
}

TAwaitVoid ProtocolSession::work() {
    co_await m_protocol->start(this);
}

TAwaitSize ProtocolSession::send(const TData& data) {
    return writer().all(data);
}

TAwaitData ProtocolSession::receive(size_t s) {
    co_await timeout(reader().all(s), RECV_TIMEOUT);
    co_return reader().ptr();
}
