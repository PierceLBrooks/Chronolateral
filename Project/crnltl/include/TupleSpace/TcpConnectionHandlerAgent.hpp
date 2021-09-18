#ifndef _TCP_CONNECTION_HANDLER_AGENT_HPP_
#define _TCP_CONNECTION_HANDLER_AGENT_HPP_

#include <TupleSpace/TcpSendAgent.hpp>
#include <TupleSpace/TcpReceiveAgent.hpp>
#include <TupleSpace/TcpListenerAgent.hpp>

class TcpConnectionHandlerAgent : Agent
{

    /***** PROPERTIES *****/
protected:
    std::vector<sf3d::TcpSocket*> mConnections;
    TcpReceiveAgent* mReceiveAgent;
    TcpSendAgent* mSendAgent;
    TcpListenerAgent* mListenAgent;
    HANDLE mBlockingListener;
    Tuple* mNewConnectionData;
    bool mHasNewConnection;


    /***** CONSTRUCTORS/DESTRUCTORS *****/
public:
    TcpConnectionHandlerAgent(unsigned short listenPort, unsigned char maxConnections);
    TcpConnectionHandlerAgent(sf3d::IpAddress address, unsigned short remotePort);
    ~TcpConnectionHandlerAgent();


    /***** METHODS *****/
public:
    void poll();
    void handle();

};


#endif
