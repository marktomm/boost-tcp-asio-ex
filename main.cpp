#include "SocketManager.h"
#include "TcpIpPort.h"

#include <memory>

typedef std::vector< boost::shared_ptr<TcpIpPort> > s_vector;

int main()
{
    SocketManager s_manager(5);

//    boost::shared_ptr<TcpIpPort> client1( new TcpIpPort("client1", &s_manager, TcpIpPort::CLIENT, (uint16_t)1000, "127.0.0.1", (uint16_t)8887));
//    boost::shared_ptr<TcpIpPort> server1( new TcpIpPort("server1", &s_manager, TcpIpPort::SERVER, (uint16_t)1000, "127.0.0.1", (uint16_t)8887));
//    boost::shared_ptr<TcpIpPort> client2( new TcpIpPort("client2", &s_manager, TcpIpPort::CLIENT, (uint16_t)1000, "127.0.0.1", (uint16_t)8889));
//    boost::shared_ptr<TcpIpPort> server2( new TcpIpPort("server2", &s_manager, TcpIpPort::SERVER, (uint16_t)1000, "127.0.0.1", (uint16_t)8889));
//    boost::shared_ptr<TcpIpPort> client3( new TcpIpPort("client3", &s_manager, TcpIpPort::CLIENT, (uint16_t)1000, "127.0.0.1", (uint16_t)8885));
//    boost::shared_ptr<TcpIpPort> server3( new TcpIpPort("server3", &s_manager, TcpIpPort::SERVER, (uint16_t)1000, "127.0.0.1", (uint16_t)8885));
//    boost::shared_ptr<TcpIpPort> client4( new TcpIpPort("client4", &s_manager, TcpIpPort::CLIENT, (uint16_t)1000, "127.0.0.1", (uint16_t)8880));
//    boost::shared_ptr<TcpIpPort> server4( new TcpIpPort("server4", &s_manager, TcpIpPort::SERVER, (uint16_t)1000, "127.0.0.1", (uint16_t)8880));

    s_vector shr_vec;

    for(int i = 0; i < 10000; i++)
    {
        {
            std::stringstream sss;
            sss <<  "server" << i;
            shr_vec.push_back( boost::shared_ptr<TcpIpPort>(new TcpIpPort(sss.str().c_str(), &s_manager, TcpIpPort::SERVER, (uint16_t)1000, "127.0.0.1", (uint16_t)8000+i)) );
        }

        {
            std::stringstream ssc;
            ssc <<  "client" << i;
            shr_vec.push_back( boost::shared_ptr<TcpIpPort> (new TcpIpPort(ssc.str().c_str(), &s_manager, TcpIpPort::CLIENT, (uint16_t)1000, "127.0.0.1", (uint16_t)8000+i)) );
        }
    }


    s_manager.Start();

    try
    {
        for(s_vector::iterator it = shr_vec.begin(); it != shr_vec.end(); ++it)
        {
            (*it)->Start();
        }
    }
    catch(std::exception &e)
    {
        std::cout << e.what() << std::endl;
    }

//    server1->Start();
//    client1->Start();

//    server2->Start();
//    client2->Start();

//    server3->Start();
//    client3->Start();

//    server4->Start();
//    client4->Start();

    s_manager.Join();

    return 0;
}

