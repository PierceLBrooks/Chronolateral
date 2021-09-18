#ifndef _TCP_SEND_AGENT_HPP_
#define _TCP_SEND_AGENT_HPP_

#include <TupleSpace/Agent.hpp>
#include <SFML3D/Network/TcpSocket.hpp>

class TcpSendAgent : public Agent
{
    friend class TcpConnectionHandlerAgent;

/***** PROPERTIES *****/
protected:
    /// Tuple retrived from TupleSpace map. Reference kept for ease of cleanup.
    Tuple* mTuple;

    /// The packet to be sent.
    sf3d::Packet* mPacket;

    /// Is mPacket ready to be sent?
    bool isReadyToSend;

    /// Number of sockets to send data out to. If client, this will always be 1.
    unsigned char mRecipientCount;

    /// Pointers to the sockets that will be used to send data.
    sf3d::TcpSocket** mSockets;

    /// Semaphore to be registered with the tuple space. Will block until packets are available to send.
    HANDLE mBlockingListener;


/***** CONSTRUCTORS/ DESTRUCTORS *****/
public:
    TcpSendAgent(unsigned char socketCount, sf3d::TcpSocket** sockets);
    ~TcpSendAgent();


/***** METHODS *****/
public:
    /// Get a single waiting packet from tuple space. Blocks until packet is available.
    void poll();

    /// Send out the packet retrieved with poll.
    void handle();

private:
    /// Add a new socket to send data to.
    void setConnections(unsigned char connectionCount, sf3d::TcpSocket** sockets);

    /// Utility method to ensure cleanup of mTuple and mPacket.
    void deleteData();

};

#endif
