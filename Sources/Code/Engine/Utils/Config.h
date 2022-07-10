#pragma once

#include "Engine/EngineModule.h"
#include "Engine/Core/Types.h"

#define JSON_NOEXCEPTION
// do this single-file include as relative pass for convenience for building system
#include "../../../ThirdParty/Json/single_include/nlohmann/json.hpp"

namespace Cyclone
{

class CConfig;
class CCommandLineParams;

ENGINE_API CConfig* GConfig();
ENGINE_API CCommandLineParams* GStartupArguments();

ENGINE_API C_STATUS GInitStartupArguments(int32 ArgC, char* ArgV[]);
ENGINE_API void GDeInitStartupArguments();

ENGINE_API C_STATUS GInitConfig(String Filename);
ENGINE_API void GDeInitConfig();

class ENGINE_API CConfig
{
public:
    DISABLE_COPY(CConfig);

    CConfig();
    ~CConfig();

    C_STATUS Init(String Path);
    void DeInit();

    // #todo_config make typed abstraction for retrieving data
    void* GetRawPtr();
    const void* GetRawPtr() const;

protected:
    class pimpl;
    UniquePtr<pimpl> m_PImpl;
};

#define GET_CONFIG_C(ConfigPtr) (*reinterpret_cast<const nlohmann::json*>((*ConfigPtr).GetRawPtr()))
#define GET_CONFIG() (*reinterpret_cast<const nlohmann::json*>((*GConfig()).GetRawPtr()))

#define GET_CONFIG_APP() GET_CONFIG()["Application"]

// Supports parameter-less args like -Arg
// And args with parameters with format -Arg=Value
// Arrays can be passed like [v1,v2,v3] without spaces
// Can be extended by implementing template specialization for ConvertFromString function
class ENGINE_API CCommandLineParams
{
public:
    C_STATUS Init(int ArgC, char* ArgV[]);
    void DeInit();

    bool ContainsParameter(String Name) const;

    template<typename T>
    bool GetParameter(String Name, T& Value) const;

public:
    int32 m_ArgC = 0;
    char** m_ArgV = nullptr;
    Vector<Pair<String, int32>> m_CachedParameters;
};

template<typename T>
bool CCommandLineParams::GetParameter(String Name, T& Value) const
{
    String NameLower(Name.size(), 0);
    for (uint32 i = 0; i < NameLower.size(); ++i)
    {
        NameLower[i] = std::tolower(Name[i]);
    }

    for (const auto& Param : m_CachedParameters)
    {

        if (NameLower == std::string_view(Param.first.begin(), Param.second >= 0 ? Param.first.begin() + Param.second : Param.first.end()))
        {
            if (Param.second < 0)
                return false;

            return ConvertFromString<T>(std::string_view(Param.first.c_str() + Param.second + 1), Value);
        }
    }

    return false;
}

} // namespace Cyclone
