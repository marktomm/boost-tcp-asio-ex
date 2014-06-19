#include "TcpIpPort.h"
#include "SocketManager.h"

using boost::uint64_t;
using boost::uint32_t;
using boost::uint16_t;
using boost::uint8_t;

using boost::int64_t;
using boost::int32_t;
using boost::int16_t;
using boost::int8_t;

SocketManager::SocketManager(uint16_t thread_cnt)
    : mIoService(),
      maWork(new boost::asio::io_service::work(mIoService)),
      mIoServiceThreads(new boost::thread_group()),
      mStarted(false)
{
    mThreadCount = thread_cnt;
}


void SocketManager::WorkerThreadFn()
{
    try
    {
        boost::system::error_code ec;
        mIoService.run( ec );
        if( ec )
        {
            boost::lock_guard<boost::mutex> lock(mStreamLock);
            std::cout << "io_service run error: " << ec;
        }
    }
    catch(std::exception& ex)
    {
        boost::lock_guard<boost::mutex> lock(mStreamLock);
        std::cout << "io_service run exception: " << ex.what();
    }
}
