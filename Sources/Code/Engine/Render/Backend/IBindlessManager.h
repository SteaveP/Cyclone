#pragma once

#include "Engine/Render/CoreRender.h"

namespace Cyclone::Render
{

using CBindlessHandle = uint32;
static const CBindlessHandle InvalidBindlessHandle = uint32(-1);

// #todo_vk_bindless this is intended to be just general buffer suballocation
struct CAllocDesc
{
    CDeviceHandle DeviceHandle;
    // #todo_vk usage?
    //uint64 ByteOffset = 0;
    uint64 ByteCount = 0;
    uint32 Alignment = 0;
};

struct CAllocHandle
{
    CHandle<CResource> Buffer;
    uint64 HostBufferByteOffset = 0;
    uint64 DeviceMemBufferDWORDOffset = 0; // offset in 4 bytes
    uint64 DeviceMemBufferDWORDCount = 0;  // count in 4 bytes

    uint64 GetDeviceMemBufferByteOffset() const { return DeviceMemBufferDWORDOffset * 4; }
};

class IBindlessManager
{
public:
    DISABLE_COPY_ENABLE_MOVE(IBindlessManager);
    IBindlessManager() = default;
    virtual ~IBindlessManager() = default;

    // Bindless
    virtual CBindlessHandle RegisterResource(CHandle<CResourceView> ResourceView) = 0;
    virtual void UnRegisterResource(CBindlessHandle Handle) = 0;

    virtual CAllocHandle AllocatePersistent(const CAllocDesc& Desc) = 0; // #todo_vk #todo_bindless this currently works as Global allocation (not temp allocation)
    virtual void FreePersistent(const CAllocHandle& Handle) = 0;

    //virtual CAllocHandle AllocateForFrame(const CAllocDesc& Desc) = 0;
    //virtual void FreeForFrame(const CAllocHandle& Handle) = 0;
};
} // namespace Cyclone::Render
