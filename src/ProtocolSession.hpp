#ifndef PROTOCOL_SESSION_HPP
#define PROTOCOL_SESSION_HPP

#include "Protocol/ITransport.hpp"

#include <Network/Session/Session.hpp>

class BaseProtocol;

class ProtocolSession : public Session, public ITransport {
public:
    ProtocolSession(Socket*);
    ~ProtocolSession();

    void setProtocol(BaseProtocol*);

private:
    TAwaitVoid work() override final;
    TAwaitSize send(const TData& data) override final;
    TAwaitData receive(size_t) override final;

    std::unique_ptr<BaseProtocol> m_protocol;
};

#endif /* PROTOCOL_SESSION_HPP */
