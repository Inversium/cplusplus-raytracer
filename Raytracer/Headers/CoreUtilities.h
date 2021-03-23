#pragma once
#include <iostream>
#include <vector>


#include "Scene.h"
#include "math/Vector.h"
#include "math/Math.h"
#include "Texture.h"

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




enum class EColorChannels : uint8_t
{
	R = 1,
	G = 2,
	B = 4,
	A = 8
};

constexpr EColorChannels operator|(const EColorChannels A, const EColorChannels B)
{
    return static_cast<EColorChannels>(static_cast<int>(A) | static_cast<int>(B));
}
constexpr EColorChannels operator&(const EColorChannels A, const EColorChannels B)
{
    return static_cast<EColorChannels>(static_cast<int>(A) & static_cast<int>(B));
}
constexpr EColorChannels operator~(const EColorChannels A)
{
    return static_cast<EColorChannels>(~static_cast<int>(A));
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

inline RTexture<Vector3> Texture3DFrom1D(const RTexture<double>& Texture, const EColorChannels Channels)
{
    RTexture<Vector3> ResultTexture(Texture.GetHeight(), Texture.GetWidth());

    for (uint32_t i = 0; i < Texture.GetHeight(); i++)
        for (uint32_t j = 0; j < Texture.GetWidth(); j++)
		{  	
	        Vector3 V;
	        if ((Channels & EColorChannels::R) == EColorChannels::R)
	            V.X = Texture.Get(j, i);
	        if ((Channels & EColorChannels::G) == EColorChannels::G)
	            V.Y = Texture.Get(j, i);
	        if ((Channels & EColorChannels::B) == EColorChannels::B)
	            V.Z = Texture.Get(j, i);
	        ResultTexture.Write(V, j, i);
		}
	
    return ResultTexture;
}