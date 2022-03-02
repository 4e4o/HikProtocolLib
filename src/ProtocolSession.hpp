#ifndef PROTOCOL_SESSION_HPP
#define PROTOCOL_SESSION_HPP

#include "Protocol/ITransport.hpp"

#include <Network/Session.hpp>

class BaseProtocol;
class QueuedSessionWriter;

class ProtocolSession : public Session, public ITransport {
public:
    ProtocolSession(boost::asio::io_context &io);
    ~ProtocolSession();

    void setProtocol(BaseProtocol*);

    boost::signals2::signal<void()> onProtocolDone;

private:
    void startImpl() override final;

    void send(const TData& data, TSendCallback c) override final;
    void receive(size_t, TReceiveCallback c) override final;

    std::unique_ptr<BaseProtocol> m_protocol;
    std::unique_ptr<QueuedSessionWriter> m_writer;
};

#endif /* PROTOCOL_SESSION_HPP */
