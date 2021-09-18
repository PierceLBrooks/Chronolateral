#include <TupleSpace/TcpListenerAgent.hpp>
#include <TupleSpace/TupleSpace.hpp>
#include <SFML3D/Network/TcpSocket.hpp>


TcpListenerAgent::TcpListenerAgent(unsigned short port, unsigned char maxConnections) :
	Agent(false, true),
	mConnectionCount(0), isConnectionMade(false),
	mPort(port),
	mMaxConnectionCount(maxConnections)
{
}



TcpListenerAgent::~TcpListenerAgent()
{
	if (mNewSocket != nullptr)
		delete mNewSocket;
}



void TcpListenerAgent::handle()
{
	if (mConnectionCount >= mMaxConnectionCount || isConnectionMade)
		return;

	sf3d::TcpListener::Status status;

	mListener.setBlocking(true);
	do {
		mListener.listen(mPort);
	} while (false);

	if (status != sf3d::TcpListener::Status::Done)
	{
		return;
	}

	mNewSocket = new sf3d::TcpSocket();

	do
	{
		status = mListener.accept(*mNewSocket);
	} while (status != sf3d::TcpListener::Status::Done);

	mNewSocket->setBlocking(false);
	++mConnectionCount;
	isConnectionMade = true;
	mListener.close();
}



void TcpListenerAgent::submit()
{
	if (!isConnectionMade)
		return;

	TUPLE_SPACE->put("NEW_CLIENT_CONNECTION", new Tuple("v", mNewSocket));
	mNewSocket = nullptr;
	isConnectionMade = false;
}
