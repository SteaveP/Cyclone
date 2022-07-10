#include "Config.h"

#include "Engine/Utils/Config.h"

#include <fstream>

namespace Cyclone
{

static UniquePtr<CConfig> GConfigPtr;
static UniquePtr<CCommandLineParams> GStartupArgumentsPtr;

CConfig* GConfig()
{
    return GConfigPtr.get();
}

CCommandLineParams* GStartupArguments()
{
    return GStartupArgumentsPtr.get();
}

C_STATUS GInitStartupArguments(int32 ArgC, char* ArgV[])
{
    GStartupArgumentsPtr = MakeUnique<CCommandLineParams>();
    C_STATUS Result = GStartupArgumentsPtr->Init(ArgC, ArgV);
    return Result;
}

void GDeInitStartupArguments()
{
    if (GStartupArgumentsPtr)
        GStartupArgumentsPtr->DeInit();

    GStartupArgumentsPtr.reset();
}

C_STATUS GInitConfig(String Filename)
{
    GConfigPtr = MakeUnique<CConfig>();
    C_STATUS Result = GConfigPtr->Init(MoveTemp(Filename));
    return Result;
}

void GDeInitConfig()
{
    if (GConfigPtr)
        GConfigPtr->DeInit();

    GConfigPtr.reset();
}

C_STATUS CCommandLineParams::Init(int ArgC, char* ArgV[])
{
    C_ASSERT_RETURN_VAL(ArgV, C_STATUS::C_STATUS_INVALID_ARG);

    m_CachedParameters.clear();

    for (uint32 i = 0; i < (uint32)ArgC; ++i)
    {
        char* EqualSign = strchr(ArgV[i], '=');
        int32 EqualSignPos = (EqualSign == nullptr ? -1 : uint32(EqualSign - ArgV[i]));

        auto& Val = m_CachedParameters.emplace_back(String(ArgV[i]), EqualSignPos);

        uint32 NameLength = uint32(EqualSignPos >= 0 ? EqualSignPos : Val.first.size());
        for (uint32 i = 0; i < NameLength; ++i)
        {
            Val.first[i] = std::tolower(Val.first[i]);
        }
    }

    return C_STATUS::C_STATUS_OK;
}

void CCommandLineParams::DeInit()
{
    m_CachedParameters.clear();
}

bool CCommandLineParams::ContainsParameter(String Name) const
{
    String NameLower(Name.size(), 0);
    for (uint32 i = 0; i < NameLower.size(); ++i)
    {
        NameLower[i] = std::tolower(Name[i]);
    }

    for (const auto& Param : m_CachedParameters)
    {
        if (NameLower == std::string_view(Param.first.begin(), Param.second >= 0 ? Param.first.begin() + Param.second : Param.first.end()))
            return true;
    }

    return false;
}

class CConfig::pimpl
{
public:
    String Path;
    nlohmann::json Json;
};

CConfig::CConfig() = default;
CConfig::~CConfig() = default;


C_STATUS CConfig::Init(String Path)
{
    m_PImpl = MakeUnique<pimpl>();
    m_PImpl->Path = Path;

    if (Path.empty() == false)
    {
        std::ifstream ConfigFile(Path);
        if (ConfigFile.good())
        {
            ConfigFile.seekg(0, std::ios_base::end);
            String JsonString(ConfigFile.tellg(), '\0');
            ConfigFile.seekg(0, std::ios_base::beg);

            ConfigFile.read(&JsonString[0], JsonString.size());

            // #todo_config parse errors
            // ConfigFile >> m_PImpl->Json;
            m_PImpl->Json = nlohmann::json::parse(JsonString, nullptr, false, true);
            C_ASSERT_RETURN_VAL(m_PImpl->Json.is_discarded() == false, C_STATUS::C_STATUS_ERROR);
        }
        else
        {
            return C_STATUS::C_STATUS_INVALID_ARG;
        }
    }

    return C_STATUS::C_STATUS_OK;
}

void CConfig::DeInit()
{
    m_PImpl.reset();
}

void* CConfig::GetRawPtr()
{
    return &m_PImpl->Json;
}

const void* CConfig::GetRawPtr() const
{
    return &m_PImpl->Json;
}

} // namespace Cyclone
