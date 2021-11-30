#ifndef CONNECTION_HPP
#define CONNECTION_HPP

#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/asio.hpp>
#include <boost/asio/deadline_timer.hpp>

#include "Protocol/ITransport.hpp"

class BaseProtocol;

class Connection
        : public boost::enable_shared_from_this<Connection>, public ITransport {
public:
    typedef boost::shared_ptr<Connection> pointer;

    Connection(boost::asio::io_context& io_context);
    ~Connection();

    void start(BaseProtocol*, const std::string& ip, int port);
    void close();

private:
    typedef std::vector<uint8_t> TData;

    void send(const TData& data, TSendCallback c) override final;
    void receive(size_t, TReceiveCallback c) override final;

    void onError();

    void onTimeout();
    void startTimeout();
    void stopTimeout();

    boost::asio::ip::tcp::socket m_socket;
    boost::asio::io_context& m_ioContext;
    std::vector<uint8_t> m_recvBuffer;
    BaseProtocol* m_protocol;
    boost::asio::deadline_timer m_timeout;
};

#endif /* CONNECTION_HPP */
