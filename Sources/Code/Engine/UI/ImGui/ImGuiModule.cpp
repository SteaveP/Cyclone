#include "ImGUIModule.h"

#include "Engine/Core/Types.h"
#include "Engine/Utils/Log.h"

#include "Engine/UI/ImGui/ImGuiRenderer.h"
#include "Engine/UI/ImGui/ImGuiPlatform.h"

#include "Engine/UI/ImGui/ImGUISubsystem.h"

namespace Cyclone
{

C_STATUS ImGUIModule::OnRegister()
{
    LOG_INFO("Module: ImGUIModule registered");

    CASSERT(GEngineGetCurrentUISubsystem() == nullptr);

    UniquePtr<ImGUISubsystem> Subsystem = MakeUnique<ImGUISubsystem>();
    Subsystem->SetPlatform(m_Platform);
    Subsystem->SetRenderer(m_Renderer);

    Cyclone::GEngineSetCurrentUISubsystem(Subsystem.release());

    return C_STATUS::C_STATUS_OK;
}

C_STATUS ImGUIModule::OnUnRegister()
{
    LOG_INFO("Module: ImGUIModule unregistered");

    Cyclone::GEngineSetCurrentUISubsystem(nullptr);

    return C_STATUS::C_STATUS_OK;
}

} // namespace Cyclone
