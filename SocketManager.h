#ifndef SOCKETMANAGER_H
#define SOCKETMANAGER_H

#include <boost/asio.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/thread.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/bind.hpp>
#include <boost/format.hpp>
#include <boost/lexical_cast.hpp>

#include <boost/cstdint.hpp>

#include <vector>
#include <list>
#include <iostream>


class SocketManager
{
private:

    boost::asio::io_service                             mIoService;
    boost::shared_ptr< boost::asio::io_service::work >  maWork;
    boost::shared_ptr< boost::thread_group >            mIoServiceThreads;

    uint16_t mThreadCount;

    bool mStarted;

    boost::mutex           mStreamLock;
    boost::recursive_mutex mResourceLock;

public:

    SocketManager(uint16_t thread_cnt = 1);

    void Start()
    {
        for(size_t i = 0; i < mThreadCount; ++i)
        {
            mIoServiceThreads->create_thread(boost::bind(&SocketManager::WorkerThreadFn, this));
        }
    }

    boost::asio::io_service& GetIoService()
    {
        return mIoService;
    }

    void Join()
    {
        mIoServiceThreads->join_all();
    }

    void AddIoServiceThreads(uint16_t cnt)
    {
        mThreadCount += cnt;

        for(size_t i = 0; i < cnt; ++i)
        {
            mIoServiceThreads->create_thread(boost::bind(&SocketManager::WorkerThreadFn, this));
        }
    }

    boost::mutex* GetStreamLock()
    {
        return &mStreamLock;
    }

    boost::recursive_mutex* GetResourceLock()
    {
        return &mResourceLock;
    }

private:

    void WorkerThreadFn();
};

#endif // SOCKETMANAGER_H
