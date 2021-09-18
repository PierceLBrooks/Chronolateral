#ifndef _LISTENER_AGENT_HPP_
#define _LISTENER_AGENT_HPP_

#include <TupleSpace/Agent.hpp>
#include <SFML3D/Network/TcpListener.hpp>
#include <SFML3D/Network/TcpSocket.hpp>

class ListenerAgent : public Agent
{
public:
    ListenerAgent();
    virtual ~ListenerAgent();

protected:
    void handle();
    void submit();
    void onShutdown();
    bool mIsInitialized;
    sf3d::TcpListener * mListener;
    Tuple * mTupleToSubmit;
};

#endif
