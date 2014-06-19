#include "SocketManager.h"
#include "TcpIpPort.h"


int main()
{
    SocketManager s_manager(2);

    boost::shared_ptr<TcpIpPort> client1( new TcpIpPort("client1", &s_manager, TcpIpPort::CLIENT, (uint16_t)1000, "127.0.0.1", (uint16_t)8887));
    boost::shared_ptr<TcpIpPort> server1( new TcpIpPort("server1", &s_manager, TcpIpPort::SERVER, (uint16_t)1000, "127.0.0.1", (uint16_t)8887));
    boost::shared_ptr<TcpIpPort> client2( new TcpIpPort("client2", &s_manager, TcpIpPort::CLIENT, (uint16_t)1000, "127.0.0.1", (uint16_t)8889));
    boost::shared_ptr<TcpIpPort> server2( new TcpIpPort("server2", &s_manager, TcpIpPort::SERVER, (uint16_t)1000, "127.0.0.1", (uint16_t)8889));
    boost::shared_ptr<TcpIpPort> client3( new TcpIpPort("client3", &s_manager, TcpIpPort::CLIENT, (uint16_t)1000, "127.0.0.1", (uint16_t)8885));
    boost::shared_ptr<TcpIpPort> server3( new TcpIpPort("server4", &s_manager, TcpIpPort::SERVER, (uint16_t)1000, "127.0.0.1", (uint16_t)8885));

    s_manager.Start();

    server1->Start();
    client1->Start();

    server2->Start();
    client2->Start();

    server3->Start();
    client3->Start();

    s_manager.Join();

    return 0;
}

