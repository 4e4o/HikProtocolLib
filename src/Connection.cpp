#include "Connection.hpp"
#include "Protocol/Base.hpp"
#include "Debug.hpp"

#include <boost/asio/ip/address.hpp>

#define RECV_BUFFER_SIZE    (64 * 1024)
#define TIMEOUT_SEC         20

using boost::asio::ip::tcp;
using boost::asio::ip::address;

Connection::Connection(boost::asio::io_context& io_context)
    : m_socket(io_context), m_ioContext(io_context),
      m_recvBuffer(RECV_BUFFER_SIZE),
      m_protocol(nullptr),
      m_timeout(io_context) {
    debug_print("Connection::Connection %p\n", this);
}

Connection::~Connection() {
    close();
    delete m_protocol;
    debug_print("Connection::~Connection %p\n", this);
}

void Connection::start(BaseProtocol* p, const std::string& ip, int port) {
    m_protocol = p;
    pointer pthis = shared_from_this();
    startTimeout();
    m_socket.async_connect(boost::asio::ip::tcp::endpoint(
                               boost::asio::ip::address::from_string(ip), port),
                           [this, pthis, p] (boost::system::error_code ec) {
        stopTimeout();
        if (ec || !p->start(this)) {
            onError();
            return;
        }
    });
}

void Connection::send(const TData& data, TSendCallback c) {
    pointer pthis = shared_from_this();

    const auto dataSize = data.size();
    boost::asio::async_write(m_socket, boost::asio::buffer(data, dataSize),
                             [c, dataSize, pthis](const boost::system::error_code& error,
                             size_t bytes_transferred) {
        if (error || (bytes_transferred != dataSize)) {
            pthis->onError();
            return;
        }

        c();
    });
}

void Connection::receive(size_t size, TReceiveCallback c) {
    if (size > RECV_BUFFER_SIZE) {
        onError();
        return;
    }

    pointer pthis = shared_from_this();
    startTimeout();
    boost::asio::async_read(m_socket, boost::asio::buffer(m_recvBuffer, size),
                            [c, pthis, size](const boost::system::error_code& error,
                            size_t bytes_transferred) {
        pthis->stopTimeout();
        if (error || (bytes_transferred != size) || !c(pthis->m_recvBuffer.data())) {
            pthis->onError();
            return;
        }
    });
}

void Connection::onError() {
    close();
    debug_print("Connection::onError %p\n", this);
}

void Connection::close() {
    try {
        m_socket.shutdown(boost::asio::ip::tcp::socket::shutdown_both);
    }  catch (...) {
    }

    try {
        m_socket.close();
    }  catch (...) {
    }
}

void Connection::onTimeout() {
    debug_print("Connection::onTimeout %p\n", this);
    close();
}

void Connection::startTimeout() {
    m_timeout.expires_from_now(boost::posix_time::seconds(TIMEOUT_SEC));
    m_timeout.async_wait([this](const boost::system::error_code& error) {
        if (error)
            return;

        onTimeout();
    });
}

void Connection::stopTimeout() {
    m_timeout.cancel();
}
