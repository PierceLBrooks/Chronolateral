#include <TupleSpace/ListenerAgent.hpp>

ListenerAgent::ListenerAgent() :
	Agent(false,true),
	mTupleToSubmit(nullptr),
	mIsInitialized(false)
{
	mListener = new sf3d::TcpListener();
	mListener->setBlocking(false);
}

ListenerAgent::~ListenerAgent()
{
	destroy();
}

void ListenerAgent::handle()
{
	if (!mIsInitialized)
	{
		if (mListener->listen(5993) != sf3d::Socket::Done)
		{
			return;
		}
		mIsInitialized = true;
	}
	sf3d::TcpSocket * socket = new sf3d::TcpSocket();
	if (mListener->accept(*socket) != sf3d::Socket::Done)
	{
		delete socket;
		return;
	}
	mTupleToSubmit = new Tuple("v", socket);
}

void ListenerAgent::submit()
{
	if (mTupleToSubmit == nullptr)
		return;
	TUPLE_SPACE->put("NEW_CONNECTION", mTupleToSubmit);
	mTupleToSubmit = nullptr;
}

void ListenerAgent::onShutdown()
{
	if (mListener == nullptr)
		return;
	mListener->close();
	delete mListener;
	mListener = nullptr;
}
