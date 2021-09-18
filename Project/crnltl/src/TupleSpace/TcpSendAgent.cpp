#include <TupleSpace/TcpSendAgent.hpp>


TcpSendAgent::TcpSendAgent(unsigned char socketCount, sf3d::TcpSocket** sockets) :
	Agent(true, false),
	isReadyToSend(false),
	mTuple(nullptr), mPacket(nullptr),
	mRecipientCount(socketCount),
	mSockets(sockets),
	mBlockingListener(CreateSemaphore(nullptr, 0, 65535, "SSURGE_TcpSendAgent_Blocker"))
{
}



TcpSendAgent::~TcpSendAgent()
{
	deleteData();
	destroy();
}



void TcpSendAgent::poll()
{
	deleteData();

	//while(WaitForSingleObject(mBlockingListener, 5000) != WAIT_ABANDONED); // blocks until a packet is available
	mTuple = TUPLE_SPACE->get("PACKET_READY");

	if (mTuple != nullptr)
	{
		mPacket = static_cast<sf3d::Packet*>(mTuple->getItemAsVoid(0));
		delete mTuple;
		mTuple = nullptr;
		isReadyToSend = true;
	}
}



void TcpSendAgent::handle()
{
	if (!isReadyToSend)
		return;

	for (unsigned char i = 0; i < mRecipientCount; ++i)
	{
		sf3d::Socket::Status sentStatus;

    sentStatus = mSockets[i]->send(*mPacket);

	}
	delete mPacket;
	mPacket = nullptr;
	isReadyToSend = false;
}



void TcpSendAgent::setConnections(unsigned char connectionCount, sf3d::TcpSocket** sockets)
{
	mSockets = sockets;
	mRecipientCount = connectionCount;
}



void TcpSendAgent::deleteData()
{
    if (mTuple != nullptr)
    {
        delete mTuple;
        mTuple = nullptr;
        mPacket = nullptr;
        isReadyToSend = false;
    }
}
