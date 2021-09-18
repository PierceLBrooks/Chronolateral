#include <TupleSpace/TcpConnectionHandlerAgent.hpp>
#include <SFML3D/Network/IpAddress.hpp>
#include <iostream>

TcpConnectionHandlerAgent::TcpConnectionHandlerAgent(unsigned short listenPort, unsigned char maxConnections) :
	Agent(true, false),
	mHasNewConnection(false),
	mNewConnectionData(nullptr),
	mBlockingListener(CreateSemaphore(nullptr, 0, maxConnections, "Client_Connection_Blocker")),
	mSendAgent(new TcpSendAgent(0, nullptr)),
	mReceiveAgent(new TcpReceiveAgent(0, nullptr))
{
	TUPLE_SPACE->registerBlockingListener(std::string("NEW_CLIENT_CONNECTION"), mBlockingListener);
	mListenAgent = new TcpListenerAgent(listenPort, maxConnections);
}



TcpConnectionHandlerAgent::TcpConnectionHandlerAgent(sf3d::IpAddress address, unsigned short remotePort) :
	Agent(false, false),
	mHasNewConnection(false),
	mNewConnectionData(nullptr), mBlockingListener(nullptr), mListenAgent(nullptr)
{
	sf3d::TcpSocket* socket = new sf3d::TcpSocket();
	sf3d::TcpSocket::Status status;

	socket->setBlocking(true);
	status = socket->connect(address, remotePort, sf3d::Time(sf3d::seconds(10)));
	if (status != sf3d::TcpSocket::Status::Done)
        std::cout << "Could not connect to the server!" << std::endl;
	mConnections.push_back(socket);
	mSendAgent = new TcpSendAgent(mConnections.size(), mConnections.data());
	mReceiveAgent = new TcpReceiveAgent(mConnections.size(), mConnections.data());
}



TcpConnectionHandlerAgent::~TcpConnectionHandlerAgent()
{
	delete mSendAgent;
	delete mReceiveAgent;
	delete mListenAgent;

	if (mNewConnectionData != nullptr)
	{
		delete mNewConnectionData;
		mNewConnectionData = nullptr;
	}
}



void TcpConnectionHandlerAgent::poll()
{
	WaitForSingleObject(mBlockingListener, INFINITE);

	mNewConnectionData = TUPLE_SPACE->get("NEW_CLIENT_CONNECTION");
	if (mNewConnectionData != nullptr)
		mHasNewConnection = true;
}



void TcpConnectionHandlerAgent::handle()
{
	if (mHasNewConnection)
	{
		mConnections.push_back(reinterpret_cast<sf3d::TcpSocket*>(mNewConnectionData->getItemAsVoid(0)));
		mSendAgent->setConnections(mConnections.size(), mConnections.data());
		mReceiveAgent->setConnections(mConnections.size(), mConnections.data());
		delete mNewConnectionData;
		mNewConnectionData = nullptr;
		mHasNewConnection = false;

	}
}
