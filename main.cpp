#include "pch.hpp"

#include "external/kiero.hpp"
#include "external/kiero_d3d9.hpp"
#include "external/kiero_intern.hpp"

#include "src/hooks/hooks.hpp"
#include "src/console/console.hpp"

#include "imgui/imgui.h"
bool want_detach = false;

using Direct3DCreate9_t = IDirect3D9 * (WINAPI*)(UINT SDKVersion);
template <> kiero::Error kiero::locate<kiero::Implementation_D3D9>(void* in, void* out)
{
    KIERO_UNUSED(in);

    auto d3d9_dll = GetModuleHandleA("d3d9.dll");
    if (!d3d9_dll) {
        KIERO_DBG_MSG("d3d9.dll not loaded");
        return Error_ModuleNotFound;
    }

    auto Direct3DCreate9 = (
        (Direct3DCreate9_t)
        GetProcAddress(d3d9_dll, "Direct3DCreate9")
        );
    if (!Direct3DCreate9) {
        KIERO_DBG_MSG("Direct3DCreate9 not found");
        return Error_MethodNotFound;
    }

    auto d3d9 = Direct3DCreate9(D3D_SDK_VERSION);
    if (!d3d9) {
        KIERO_DBG_MSG("Direct3DCreate9 failed");
        return Error_D3D9_Direct3DCreate9Failed;
    }
    KIERO_DEFER([&]() { d3d9->Release(); });

    kiero::_::DummyWin32Window window;
    kiero::_::create_dummy_win32_window(&window);
    KIERO_DEFER([&]() { kiero::_::destroy_dummy_win32_window(&window); });

    D3DPRESENT_PARAMETERS present_parameters = {};
    present_parameters.Windowed = TRUE;
    present_parameters.SwapEffect = D3DSWAPEFFECT_DISCARD;
    present_parameters.hDeviceWindow = window.hwnd;

    IDirect3DDevice9* device;
    auto hresult = d3d9->CreateDevice(
        D3DADAPTER_DEFAULT,
        D3DDEVTYPE_HAL,
        window.hwnd,
        D3DCREATE_SOFTWARE_VERTEXPROCESSING,
        &present_parameters,
        &device
    );
    if (hresult != S_OK) {
        KIERO_DBG_MSG("CreateDevice failed (%d)", hresult);
        return Error_D3D9_CreateDeviceFailed;
    }
    KIERO_DEFER([&]() { device->Release(); });

    D3D9Output* output = (D3D9Output*)out;

    for (auto vtable = *(void***)device; vtable; vtable++) {
        auto ptr = *vtable;
        if (!ptr) break;
        output->device_methods.push_back(ptr);
    }

    return Error_Nil;
}

void unload()
{
    if (ImGui::IsKeyDown(ImGuiKey_F1) && ImGui::IsKeyPressed(ImGuiKey_F2, false)) {
        want_detach = true;
        con::print("tounel: unloading", colors::yellow, colors::reset);
        RemoveHooks();
        FreeLibraryAndExitThread(GetModuleHandleW(nullptr), 0);
    }
}
BOOL APIENTRY DllMain(HMODULE module, DWORD reason, LPVOID reserved)
{
    switch (reason) {
    case DLL_PROCESS_ATTACH:
        AllocConsole();
        FILE* f;
		freopen_s(&f, "CONOUT$", "w", stdout);
        con::print("tounel: attached", colors::green, colors::reset);
        DisableThreadLibraryCalls(module);
        StartHookThread();
        break;
    case DLL_PROCESS_DETACH:
            if (reserved == nullptr) RemoveHooks();
            con::print("tounel: detached", colors::red, colors::reset);
            break;
        
    default:
        break;
    }
    return TRUE;
}
