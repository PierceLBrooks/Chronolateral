#ifndef _TCP_RECEIVE_AGENT_HPP_
#define _TCP_RECEIVE_AGENT_HPP_

#include <TupleSpace/Agent.hpp>
#include <SFML3D/Network/TcpSocket.hpp>
#include <SFML3D/Network/Packet.hpp>

class TcpReceiveAgent : public Agent
{
    friend class TcpConnectionHandlerAgent;

/****** PROPERTIES *****/
protected:
    /// Contains received data before it is processed by DisassemblerAgent
    sf3d::Packet mPacket;

    /// List of connections that will be sending data to this agent.
    sf3d::TcpSocket** mSockets;

    /// Number of socket connections.
    unsigned char mSenderCount;

    /// Does the packet contain data that is ready to be pushed to TupleSpace?
    bool isReadyToPush;


/***** CONSTRUCTORS / DESTRUCTORS *****/
public:
    TcpReceiveAgent(unsigned char senderCount, sf3d::TcpSocket** sockets);
    ~TcpReceiveAgent();


/***** METHODS *****/
public:
    /// Retrieves data from connections.
    void handle();

    /// Sends retrieved data to TupleSpace.
    void submit();

private:
    /// Adds a new connection that this agent could receive data from.
    void setConnections(unsigned char connectionCount, sf3d::TcpSocket** sockets);
};

#endif
