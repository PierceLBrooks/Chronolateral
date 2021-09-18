#ifndef _TUPLE_SPACE_HPP_
#define _TUPLE_SPACE_HPP_

#include <TupleSpace/Tuple.hpp>
#include <windows.h>
#include <map>
#include <string>
#include <queue>
#include <mutex>

#define TUPLE_SPACE TupleSpace::getSingletonPtr()

class TupleSpace
{
public:
    TupleSpace();
    virtual ~TupleSpace();
    void put(const std::string& tag, Tuple* t);
    Tuple* get(const std::string& tag);

    /// Registers a semaphore with the listener registry. This semaphore will be incremented whenever a new element is added to the tuple space using the corresponding tag.
    bool registerBlockingListener(std::string tag, HANDLE semaphore);

    /// Unregisters and deletes a semaphore from the listener registry.
    void unregisterBlockingListener(std::string tag);

    static TupleSpace* getSingletonPtr();


protected:
    std::map<std::string, std::queue<Tuple*>*> mSpace;
    std::mutex mMutex;

    /// Keeps track of blocking listeners (semaphores) to be incremented when elements with their corresponding tags are added to the tuple space.
    std::map<std::string, HANDLE> mListenerRegistry;

    static TupleSpace* mSingletonPtr;
};

#endif
