#include <TupleSpace/Agent.hpp>
#include <iostream>

Agent::Agent(bool doesPoll, bool doesSubmit) :
	mIsRunning(true),
	mIsShutdown(false),
	mDoesPoll(doesPoll),
	mDoesSubmit(doesSubmit)
{
	mThread = new std::thread(&Agent::run,this);
}

Agent::~Agent()
{
	destroy();
}

void Agent::shutdown()
{
	//std::unique_lock<std::mutex> lock(mMutex);
	mIsRunning = false;
	onShutdown();
}

bool Agent::getIsShutdown()
{
	//std::unique_lock<std::mutex> lock(mMutex);
	return mIsShutdown;
}

void Agent::run()
{
	while (true)
	{
		//{
			//std::unique_lock<std::mutex> lock(mMutex);
			if (!mIsRunning)
			{
				break;
			}
		//}
		if (mDoesPoll)
		{
			poll();
		}
		handle();
		if (mDoesSubmit)
		{
			submit();
		}
		//{
			//std::unique_lock<std::mutex> lock(mMutex);
			if (!mIsRunning)
			{
				break;
			}
		//}
		std::this_thread::sleep_for(std::chrono::microseconds(AGENT_SLEEP_TIME));
	}
	//{
		//std::unique_lock<std::mutex> lock(mMutex);
		mIsShutdown = true;
	//}
}

void Agent::destroy()
{
	if (mThread == nullptr)
		return;
    shutdown();
	while (mThread->joinable())
		mThread->join();
	delete mThread;
	mThread = nullptr;
}

void Agent::poll()
{

}

void Agent::handle()
{

}

void Agent::submit()
{

}

void Agent::onShutdown()
{

}
