#pragma once

#ifdef DYNAMIC_LIB
#ifdef DYNAMIC_LIB_DX12
#define RENDER_BACKEND_DX12_API DLL_EXPORT
#else
#define RENDER_BACKEND_DX12_API DLL_IMPORT
#endif
#else
#define RENDER_BACKEND_DX12_API
#endif
