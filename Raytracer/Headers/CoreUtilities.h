#pragma once
#include <iostream>
#include <vector>

#include "math/Vector.h"
#include "math/Math.h"

enum class LogType : uint8_t
{
    LOG,
    WARNING,
    ERROR
};


template<typename... Args>
inline void LOG(const char* Context, LogType Type, const char* Message, Args... args)
{
    std::string MessageType = "[]";
    switch (Type)
    {
    case LogType::LOG:
        MessageType.insert(1, "LOG");
        break;
    case LogType::WARNING:
        MessageType.insert(1, "WARNING");
        break;
    case LogType::ERROR:
        MessageType.insert(1, "ERROR");
        break;
    }
    char Buffer[300];
    std::sprintf(Buffer, (MessageType + "[" + std::string(Context) + "]\t" + std::string(Message)).c_str(), args...);
    std::cout << Buffer << std::endl;
} 



inline void DrawPercent(const char* Context, const char* Task, const uint32_t Current, const uint32_t Max, const uint8_t Step)
{
    const uint8_t Denom = 100 / Step;
    if (Current % (Max / Denom) == 0)
    {
        uint32_t Percent = static_cast<uint32_t>(static_cast<double>(Current) / Max * 100);
        LOG(Context, LogType::LOG, (std::string(Task) + "... %d%%").c_str(), Percent);
    }
}
inline void DrawTask(const char* Context, const char* Task = "Processing")
{
    LOG(Context, LogType::LOG, Task);
}
