#include "TcpIpPort.h"
#include "SocketManager.h"

#include <stdio.h>      /* printf, scanf, puts, NULL */
#include <stdlib.h>     /* srand, rand */
#include <time.h>       /* time */



TcpIpPort::TcpIpPort(const char* name, SocketManager* sm, PortType type, uint16_t buf_size, char *addr, uint16_t port)
    : mSocket(sm->GetIoService()),
      mAcceptor(sm->GetIoService()),
      mStrand(sm->GetIoService()),
      mpManager(sm),
      mType(type),
      mAddress(addr),
      mPort(port),
      mBuffSize(buf_size),
      mName(name),
      mTimer(sm->GetIoService(), boost::posix_time::milliseconds(mDelay)),
      mDelay(DELAY_DEFAULT)
{
    // each thread needs a a separate random seed.
    srand(time(NULL));
}

void TcpIpPort::Start()
{
//    mStrand.post(boost::bind(&TcpIpPort::AsyncStart, shared_from_this()));
    mpManager->GetIoService().post(boost::bind(&TcpIpPort::AsyncStart, shared_from_this()));
}



void TcpIpPort::AsyncStart()
{
    try
    {
        mTimer.expires_from_now(boost::posix_time::milliseconds(mDelay));

        switch(mType)
        {
        case CLIENT:
        {
            boost::asio::ip::tcp::resolver resolver( mpManager->GetIoService() );
            boost::asio::ip::tcp::resolver::query query(
                mAddress.c_str(),
                boost::lexical_cast< std::string >( mPort )
            );
            boost::asio::ip::tcp::resolver::iterator iterator = resolver.resolve( query );
            boost::asio::ip::tcp::endpoint endpoint = *iterator;

            mSocket.async_connect( endpoint, boost::bind( &TcpIpPort::OnConnect, shared_from_this(), _1) );

            break;
        }
        case SERVER:
        {
            boost::asio::ip::tcp::resolver resolver( mpManager->GetIoService() );
            boost::asio::ip::tcp::resolver::query query(
                mAddress.c_str(),
                boost::lexical_cast< std::string >( mPort )
            );
            boost::asio::ip::tcp::endpoint endpoint = *resolver.resolve( query );
            mAcceptor.open( endpoint.protocol() );
            mAcceptor.set_option( boost::asio::ip::tcp::acceptor::reuse_address( false ) );
            mAcceptor.bind( endpoint ); // bind rarely throws exception here
            mAcceptor.listen( boost::asio::socket_base::max_connections );
            mAcceptor.async_accept( mSocket, boost::bind( &TcpIpPort::OnAccept, shared_from_this(), _1) );

            break;
        }
        }
    }
    catch(std::exception& ex)
    {
        boost::lock_guard<boost::mutex> lock(*mpManager->GetStreamLock());
        std::cout << "TcpIpPort Start exception: " << ex.what();
    }
}



void TcpIpPort::GenerateMessage()
{
    mMessageLen = rand() % 30 + 10;
    //boost::this_thread::sleep(boost::posix_time::seconds(rand() % 5 + 1));

    static const char alphanum[] =
        "0123456789"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz";

    for(std::size_t i = 0; i < mMessageLen; ++i)
    {
        mBuffWrite += alphanum[rand() % (sizeof(alphanum) - 1)];;
    }

    boost::lock_guard<boost::mutex> lock(*mpManager->GetStreamLock());
    std::cout << mName << " : ""[" << boost::this_thread::get_id() << "]" <<  "Message generated: " << mBuffWrite << std::endl;
}

void TcpIpPort::OnConnect(const boost::system::error_code & ec)
{
    if( ec )
    {
        {
        boost::lock_guard<boost::mutex> lock(*mpManager->GetStreamLock());
        std::cout << mName << " : ""[" << boost::this_thread::get_id() << "]" << "TcpIpPort OnConnect error: " << ec.message()
                  << " Retry in " << mDelay/1000 << " seconds." << std::endl;
        }

        if(mDelay < DELAY_MAX)
            mDelay *= 2;

        mTimer.async_wait(boost::bind(&TcpIpPort::AsyncStart, shared_from_this()));
//        mSocket.close();
    }
    else
    {
        mStrand.post(boost::bind(&TcpIpPort::ClientDoWrite, shared_from_this()));
    }
}

void TcpIpPort::ClientDoWrite()
{
    if(mBuffWrite.empty())
    {
        if(!mBuffRead.empty())
        {
            ReadBufferContentsToStdOut();
            ClearReadBuffer();
        }
        else
        {
            GenerateMessage();

            // Simulate fragmentary message sending from client side
            uint8_t size_to_send = rand() % mMessageLen + 1;
            mMessageLen -= size_to_send;

            mSocket.async_write_some(boost::asio::buffer(mBuffWrite.c_str(), size_to_send/*mBuffWrite.size()*/),
                                     boost::bind(&TcpIpPort::ClientOnWrite, shared_from_this(), _1, _2));
        }
    }
    else
    {
        mSocket.async_write_some(boost::asio::buffer(mBuffWrite.c_str(), mMessageLen /*mBuffWrite.size()*/),
                                 boost::bind(&TcpIpPort::ClientOnWrite, shared_from_this(), _1, _2));
    }
}

void TcpIpPort::ClientOnWrite(const boost::system::error_code & ec, size_t bytes)
{
    if(ec)
    {
        boost::lock_guard<boost::mutex> lock(*mpManager->GetStreamLock());
        std::cout << mName << " : ""[" << boost::this_thread::get_id() << "]" <<
                     "TcpIpPort ClientOnWrite error: " << ec.message() << std::endl;

        mSocket.close();
    }
    else
    {
        mBuffWrite.erase(0, bytes);

        if(!mBuffWrite.empty())
        {
            ClientDoWrite();
        }
        else
        {
            mStrand.post(boost::bind(&TcpIpPort::ClientDoRead, shared_from_this()));
        }
    }
}

void TcpIpPort::ClientDoRead()
{
    mSocket.async_read_some(boost::asio::buffer(read_buffer_),
                            boost::bind(&TcpIpPort::ClientOnRead, shared_from_this(), _1, _2));
}

void TcpIpPort::ClientOnRead(const boost::system::error_code & ec, size_t bytes)
{
    if(ec)
    {
        boost::lock_guard<boost::mutex> lock(*mpManager->GetStreamLock());
        std::cout << mName << " : ""[" << boost::this_thread::get_id() << "]" <<
                     "TcpIpPort ClientOnWrite error: " << ec.message() << std::endl;

        mSocket.close();
    }
    else
    {

        for(size_t i = 0; i < bytes; ++i)
        {
            mBuffRead += read_buffer_[i];
        }


        {
            boost::lock_guard<boost::mutex> lock(*mpManager->GetStreamLock());
            std::cout << mName << " : ""[" << boost::this_thread::get_id() << "]" <<  "Client Got: " << mBuffRead << std::endl;
        }

        ClearReadBuffer();

        mStrand.post(boost::bind(&TcpIpPort::ClientDoWrite, shared_from_this()));
    }
}

void TcpIpPort::OnAccept(const boost::system::error_code & ec)
{
    if( ec )
    {
        boost::lock_guard<boost::mutex> lock(*mpManager->GetStreamLock());
        std::cout << mName << " : ""[" << boost::this_thread::get_id() << "]" <<
                     "TcpIpPort OnAccept error: " << ec.message() << std::endl;
        mSocket.close();
    }
    else
    {
        mStrand.post(boost::bind(&TcpIpPort::ServerDoRead, shared_from_this()));
    }
}

void TcpIpPort::ServerDoRead()
{
    if(!mBuffRead.empty())
    {
        mBuffWrite += mBuffRead;
    }

    mSocket.async_read_some(boost::asio::buffer(read_buffer_),
                            boost::bind(&TcpIpPort::ServerOnRead, shared_from_this(), _1, _2));
}

void TcpIpPort::ServerOnRead(const boost::system::error_code &ec, size_t bytes)
{
    if(ec)
    {
        boost::lock_guard<boost::mutex> lock(*mpManager->GetStreamLock());
        std::cout << mName << " : ""[" << boost::this_thread::get_id() << "]" <<
                     "TcpIpPort ServerOnRead error: " << ec.message() << std::endl;
        mSocket.close();
    }
    else
    {
        mMessageLen = bytes;

        for(size_t i = 0; i < mMessageLen; ++i)
        {
            mBuffWrite += read_buffer_[i];
        }

        {
            boost::lock_guard<boost::mutex> lock(*mpManager->GetStreamLock());
//            std::cout << mName << " : ""[" << boost::this_thread::get_id() << "]" <<  "Server Got: " << mBuffWrite << std::endl;
        }

        mStrand.post(boost::bind(&TcpIpPort::ServerDoWrite, shared_from_this()));
    }
}

void TcpIpPort::ServerDoWrite()
{
    if(!mBuffWrite.empty())
    {   
        // Simulate fragmentary message sending from server
        uint8_t size_to_send = rand() % mMessageLen + 1;
        mMessageLen -= size_to_send;

        mSocket.async_write_some(boost::asio::buffer(mBuffWrite.c_str(), size_to_send/*mBuffWrite.size()*/),
                                 boost::bind(&TcpIpPort::ServerOnWrite, shared_from_this(), _1, _2));
    }
    else
    {
        mSocket.async_read_some(boost::asio::buffer(read_buffer_),
                                boost::bind(&TcpIpPort::ServerOnRead, shared_from_this(), _1, _2));
    }
}

void TcpIpPort::ServerOnWrite(const boost::system::error_code &ec, size_t bytes)
{
    if(ec)
    {
        boost::lock_guard<boost::mutex> lock(*mpManager->GetStreamLock());
        std::cout << mName << " : ""[" << boost::this_thread::get_id() << "]" <<
                     "TcpIpPort ServerOnWrite error: " << ec.message() << std::endl;
        mSocket.close();
    }
    else
    {
        mBuffWrite.erase(0, bytes);

        if(!mBuffWrite.empty())
        {
            mSocket.async_write_some(boost::asio::buffer(mBuffWrite.c_str(), mBuffWrite.size()),
                                     boost::bind(&TcpIpPort::ServerOnWrite, shared_from_this(), _1, _2));
        }
        else
        {
            mStrand.post(boost::bind(&TcpIpPort::ServerDoRead, shared_from_this()));
        }
    }
}

inline void TcpIpPort::ReadBufferContentsToStdOut()
{
    boost::lock_guard<boost::mutex> lock(*mpManager->GetStreamLock());
    std::cout << mName << " : ""[" << boost::this_thread::get_id() << "]" <<  "ReadBufferContentsToStdOut(): " << mBuffRead << std::endl;
}

inline void TcpIpPort::ClearReadBuffer()
{
    mBuffRead.clear();
}

inline void TcpIpPort::ClearWriteBuffer()
{
    mBuffWrite.clear();
}
