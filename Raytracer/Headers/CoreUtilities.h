#pragma once
#include <iostream>
#include <format>

#include "math/Vector.h"
#include "math/Math.h"

enum class LogType : uint8_t
{
    LOG,
    WARNING,
    ERROR
};


template<typename... Args>
inline void LOG(const std::string& Context, LogType Type, const std::string& Message, Args... args)
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

    std::string Output = std::format(MessageType + "[" + std::string(Context) + "]\t" + std::string(Message), args...);
    #pragma omp critical
    {      
        std::cout << Output << std::endl;
    }
} 



inline void DrawPercent(const std::string& Context, const std::string& Task, const uint32_t Current, const uint32_t Max, const uint8_t Step)
{
    const uint8_t Denom = 100 / Step;
    if (Current % (Max / Denom) == 0)
    {
        uint32_t Percent = static_cast<uint32_t>(static_cast<double>(Current) / Max * 100);
        LOG(Context, LogType::LOG, (std::string(Task) + "... {}%"), Percent);
    }
}
inline void DrawTask(const std::string& Context, const std::string& Task = "Processing")
{
    LOG(Context, LogType::LOG, Task);
}
