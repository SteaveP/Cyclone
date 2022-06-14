#include "ImGUIModule.h"

#include "Engine/Core/Types.h"

#include "Engine/UI/ImGui/ImGuiRenderer.h"
#include "Engine/UI/ImGui/ImGuiPlatform.h"

#include "Engine/UI/ImGui/ImGUISubsystem.h"

namespace Cyclone
{

C_STATUS ImGUIModule::OnRegister()
{
#ifdef _DEBUG
    printf("Module: ImGUIModule registered\n");
#endif

    CASSERT(GEngineGetCurrentUISubsystem() == nullptr);

    UniquePtr<ImGUISubsystem> Subsystem = std::make_unique<ImGUISubsystem>();
    Subsystem->SetPlatform(m_Platform);
    Subsystem->SetRenderer(m_Renderer);

    Cyclone::GEngineSetCurrentUISubsystem(Subsystem.release());

    return C_STATUS::C_STATUS_OK;
}

C_STATUS ImGUIModule::OnUnRegister()
{
#ifdef _DEBUG
    printf("Module: ImGUIModule unregistered\n");
#endif

    Cyclone::GEngineSetCurrentUISubsystem(nullptr);

    return C_STATUS::C_STATUS_OK;
}

} // namespace Cyclone
