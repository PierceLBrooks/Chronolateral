#ifndef _TCP_LISTENER_AGENT_HPP_
#define _TCP_LISTENER_AGENT_HPP_

#include <TupleSpace/Agent.hpp>
#include <SFML3D/Network/TcpListener.hpp>

class TcpListenerAgent : public Agent
{
    /***** PROPERTIES *****/
protected:

    /// Listens for clients attempting to connect.
    sf3d::TcpListener mListener;

    /// Holds socket data for connected clients until it can be submitted to the TupleSpace.
    sf3d::TcpSocket* mNewSocket;

    /// Is a client connection waiting to be submitted?
    bool isConnectionMade;

    /// The port to listen on.
    unsigned short mPort;

    /// The maximum number of client connections allowed.
    unsigned char mMaxConnectionCount;

    /// The current number of client connections.
    unsigned char mConnectionCount;


    /***** CONSTRUCTORS / DESTRUCTORS *****/
public:
    TcpListenerAgent(unsigned short port, unsigned char maxConnections);
    ~TcpListenerAgent();


    /***** METHODS *****/
public:
    /// Listen for new connections and accept a new socket if found.
    void handle();

    /// Submit the new client connection to the TupleSpace.
    void submit();
};

#endif
