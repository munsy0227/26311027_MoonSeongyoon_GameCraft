#include "GameTime.h"
#include <chrono>

namespace
{
    long long NowMilliseconds()
    {
        return std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::steady_clock::now().time_since_epoch()).count();
    }
}

int GameTime::Init()
{
    begintime = NowMilliseconds();
    eplapsedTime = deltaTime = 0;
    return 0;
}

int GameTime::Update()
{
    const auto previous = eplapsedTime;
    eplapsedTime = NowMilliseconds() - begintime;
    deltaTime = eplapsedTime - previous;
    return 0;
}
