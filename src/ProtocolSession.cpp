#include "ProtocolSession.hpp"
#include "Protocol/Base.hpp"
#include "Network/QueuedSessionWriter.hpp"

using boost::signals2::connection;

ProtocolSession::ProtocolSession(boost::asio::io_context &io)
    : Session(io),
      m_writer(new QueuedSessionWriter(this)) {
}

ProtocolSession::~ProtocolSession() {
    onProtocolDone();
}

void ProtocolSession::setProtocol(BaseProtocol* p) {
    m_protocol.reset(p);
}

void ProtocolSession::startImpl() {
    m_protocol->start(this);
}

void ProtocolSession::send(const TData& data, TSendCallback c) {
    onWriteDone.connect_extended([c](const connection& con) {
        con.disconnect();
        c();
    });
    m_writer->writeAll(data.data(), data.size());
}

void ProtocolSession::receive(size_t s, TReceiveCallback c) {
    onData.connect_extended([c](const connection& con, const uint8_t *ptr, std::size_t) {
        con.disconnect();
        c(ptr);
    });
    readAll(s);
}
