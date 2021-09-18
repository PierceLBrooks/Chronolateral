#include <TupleSpace/TupleSpace.hpp>
#include <SFML3D/Network/Packet.hpp>

TupleSpace* TupleSpace::mSingletonPtr = nullptr;

TupleSpace::TupleSpace()
{
    mSingletonPtr = this;
}

TupleSpace::~TupleSpace()
{
	for (std::map<std::string, std::queue<Tuple*>*>::iterator iter = mSpace.begin(); iter != mSpace.end(); ++iter)
	{
		while (!iter->second->empty())
		{
			delete iter->second->front();
			iter->second->pop();
		}
		delete iter->second;
	}
	mSpace.clear();
}

TupleSpace* TupleSpace::getSingletonPtr()
{
    return mSingletonPtr;
}

void TupleSpace::put(const std::string& tag, Tuple * t)
{
	mMutex.lock();
	std::map<std::string, std::queue<Tuple*>*>::iterator iter = mSpace.find(tag);
	if (iter == mSpace.end())
		mSpace[tag] = new std::queue<Tuple*>();
	mSpace[tag]->push(t);

	if (mListenerRegistry.find(tag) != mListenerRegistry.end())
		ReleaseSemaphore(mListenerRegistry[tag], 1, nullptr);

	mMutex.unlock();
}

Tuple * TupleSpace::get(const std::string& tag)
{
	mMutex.lock();
	std::map<std::string, std::queue<Tuple*>*>::iterator iter = mSpace.find(tag);
	if (iter == mSpace.end())
	{
		mMutex.unlock();
		return nullptr;
	}
	Tuple * t = iter->second->front();
	iter->second->pop();
	if (iter->second->empty())
	{
		delete iter->second;
		mSpace.erase(iter);
	}
	mMutex.unlock();
	return t;
}

bool TupleSpace::registerBlockingListener(std::string tag, HANDLE semaphore)
{
	mMutex.lock();

	if (mListenerRegistry.find(tag) == mListenerRegistry.end())
	{
		mListenerRegistry.insert(std::pair<std::string, HANDLE>(tag, semaphore));
		mMutex.unlock();
		return true;
	} else
	{
		mMutex.unlock();
		return false;
	}
}

void TupleSpace::unregisterBlockingListener(std::string tag)
{
	mMutex.lock();

	std::map<std::string, HANDLE>::const_iterator loc = mListenerRegistry.find(tag);

	if(loc != mListenerRegistry.end())
		mListenerRegistry.erase(loc);

	mMutex.unlock();
}
