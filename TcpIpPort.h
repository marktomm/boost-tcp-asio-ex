#ifndef TCPIPPORT_H
#define TCPIPPORT_H

#include <boost/asio.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/thread.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/bind.hpp>
#include <boost/format.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/enable_shared_from_this.hpp>

#include <boost/cstdint.hpp>

#include <vector>
#include <list>

class SocketManager;

class TcpIpPort : public boost::enable_shared_from_this<TcpIpPort>
{
public:

    enum PortType
    {
        CLIENT,
        SERVER
    };

    enum ConnectionDelay
    {
        DELAY_DEFAULT = 1000,
        DELAY_MAX = 8000
    };

private:

    boost::asio::ip::tcp::socket    mSocket;
    boost::asio::ip::tcp::acceptor  mAcceptor;
    boost::asio::io_service::strand mStrand;

    SocketManager* mpManager;

    PortType mType;

    std::string mAddress;
    uint16_t    mPort;

    std::string mBuffRead;
    std::string mBuffWrite;

    uint16_t    mBuffSize;
    uint16_t    mMessageLen;
    std::string mName;

    boost::mutex mBuffWriteLock;
    boost::mutex mBuffReadLock;

    enum { max_msg = 1024 };
    char read_buffer_[max_msg];
    char write_buffer_[max_msg];

    boost::asio::deadline_timer mTimer;
    uint16_t mDelay;

public:

    TcpIpPort(const char *name, SocketManager* sm, PortType type, uint16_t buf_size = 1024, char *addr = "127.0.0.1", uint16_t port = 80);

    void Start();
    void AsyncStart();

    void GenerateMessage();

private:

    void OnConnect(const boost::system::error_code & ec);
    void ClientDoWrite();
    void ClientOnWrite(const boost::system::error_code & ec, size_t bytes);
    void ClientDoRead();
    void ClientOnRead(const boost::system::error_code & ec, size_t bytes);

    void OnAccept(const boost::system::error_code & ec);
    void ServerDoRead();
    void ServerOnRead(const boost::system::error_code & ec, size_t bytes);
    void ServerDoWrite();
    void ServerOnWrite(const boost::system::error_code & ec, size_t bytes);

    void ReadBufferContentsToStdOut();
    void ClearReadBuffer();
    void ClearWriteBuffer();
};


#endif // TCPIPPORT_H
