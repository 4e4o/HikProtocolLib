#ifndef CONNECTION_HPP
#define CONNECTION_HPP

#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/asio.hpp>

#include "Protocol/ITransport.hpp"
#include "Misc/Timer.hpp"

class BaseProtocol;

class Connection
        : public boost::enable_shared_from_this<Connection>, public ITransport, public Timer {
public:
    typedef boost::shared_ptr<Connection> pointer;
    typedef std::function<void()> TDestroyCallback;

    Connection(boost::asio::io_context& io_context);
    ~Connection();

    void start(BaseProtocol*, const std::string& ip, int port);
    void close();

    void setOnDestroy(TDestroyCallback);

private:
    typedef std::vector<uint8_t> TData;

    void send(const TData& data, TSendCallback c) override final;
    void receive(size_t, TReceiveCallback c) override final;
    void onTimeout() override final;

    void onError();

    boost::asio::ip::tcp::socket m_socket;
    std::vector<uint8_t> m_recvBuffer;
    std::unique_ptr<BaseProtocol> m_protocol;
    TDestroyCallback m_onDestroy;
};

#endif /* CONNECTION_HPP */
