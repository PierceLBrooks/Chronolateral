#ifndef _AGENT_HPP_
#define _AGENT_HPP_

#include <TupleSpace/TupleSpace.hpp>
#include <thread>
#include <mutex>

#define AGENT_SLEEP_TIME 10

class Agent
{
public:
    Agent(bool doesPoll, bool doesSubmit);
    virtual ~Agent();
    void shutdown();
    bool getIsShutdown();

protected:
    void run();
    void destroy();
    virtual void poll();
    virtual void handle();
    virtual void submit();
    virtual void onShutdown();
    bool mIsRunning;
    bool mIsShutdown;
    bool mDoesPoll;
    bool mDoesSubmit;
    std::thread * mThread;
    std::mutex mMutex;
};

#endif
