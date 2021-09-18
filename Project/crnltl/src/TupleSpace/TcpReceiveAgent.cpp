#include <TupleSpace/TcpReceiveAgent.hpp>
#include <TupleSpace/TupleSpace.hpp>

TcpReceiveAgent::TcpReceiveAgent(unsigned char socketCount, sf3d::TcpSocket** sockets) :
	Agent(false, true),
	isReadyToPush(false),
	mSenderCount(socketCount),
	mSockets(sockets)
{
}



TcpReceiveAgent::~TcpReceiveAgent()
{
	for (unsigned char i = 0; i < mSenderCount; ++i)
    {
        delete mSockets[i];
    }
}



void TcpReceiveAgent::handle()
{
	if (isReadyToPush)
		return;


	mPacket = sf3d::Packet();

	for (unsigned char i = 0; i < mSenderCount; ++i)
	{
	    unsigned int length = 0;
	    char* buffer = new char[1024];
		sf3d::TcpSocket::Status status;

		do
		{
			status = mSockets[i]->receive(buffer,1024,length);
		} while (status != sf3d::TcpSocket::Status::Done);

		if (status == sf3d::TcpSocket::Status::Done)
		{
            mPacket.append(buffer,length);
            delete[] buffer;
			isReadyToPush = true;
			break;
		}
		delete[] buffer;
	}
}



void TcpReceiveAgent::submit()
{
	if (!isReadyToPush)
		return;

	sf3d::Packet* pack = new sf3d::Packet();
	pack->append(mPacket.getData(),mPacket.getDataSize());
	//memcpy_s(pack, sizeof(sf3d::Packet), &mPacket, sizeof(sf3d::Packet));
	TUPLE_SPACE->put("RECEIVE_PACKET", new Tuple("v", pack));
	isReadyToPush = false;
}



void TcpReceiveAgent::setConnections(unsigned char connectionCount, sf3d::TcpSocket** sockets)
{
	mSockets = sockets;
	mSenderCount = connectionCount;
}
