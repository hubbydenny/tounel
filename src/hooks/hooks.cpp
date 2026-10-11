#include "pch.hpp"
#include "hooks.hpp"

#include "sdk/interfaces/interfaces.hpp"

#include "src/menu/menu.hpp"

namespace {

using EndScene_t = HRESULT(STDMETHODCALLTYPE*)(IDirect3DDevice9*);
using Reset_t = HRESULT(STDMETHODCALLTYPE*)(IDirect3DDevice9*, D3DPRESENT_PARAMETERS*);

EndScene_t oEndScene = nullptr;
Reset_t oReset = nullptr;
void* g_target = nullptr;
void* g_reset_target = nullptr;
HANDLE g_hook_thread = nullptr;
bool g_minhook_owned = false;

HRESULT STDMETHODCALLTYPE hkEndScene(IDirect3DDevice9* device)
{
    const bool menu_ready = tounel::menu::Initialize(device);

    if (menu_ready) {
        tounel::menu::RenderFrame();
    }

    const HRESULT hr = oEndScene(device);

    if (menu_ready && (hr == D3DERR_DEVICELOST || hr == D3DERR_DEVICEREMOVED)) {
        tounel::menu::InvalidateDevice();
    }
    return hr;
}

HRESULT STDMETHODCALLTYPE hkReset(IDirect3DDevice9* device, D3DPRESENT_PARAMETERS* params)
{
    tounel::menu::InvalidateDevice();

    const HRESULT hr = oReset(device, params);

    if (SUCCEEDED(hr)) {
        tounel::menu::RecreateDevice();
    }
    return hr;
}

bool WaitForD3D9(DWORD timeout_ms)
{
    for (DWORD waited = 0; waited < timeout_ms; waited += kD3D9PollIntervalMs) {
        if (GetModuleHandleW(L"d3d9.dll")) {
            return true;
        }
        Sleep(kD3D9PollIntervalMs);
    }
    return GetModuleHandleW(L"d3d9.dll") != nullptr;
}

bool InstallEndSceneHook()
{
    kiero::D3D9Output output;
    if (kiero::locate<kiero::Implementation_D3D9>(nullptr, &output) != kiero::Error_Nil) {
        KIERO_DBG_MSG("tounel: kiero::locate<D3D9> failed");
        return false;
    }

    if (output.device_methods.size() <= kEndSceneVtableIndex ||
        output.device_methods.size() <= kResetVtableIndex) {
        KIERO_DBG_MSG("tounel: vtable dump too small");
        return false;
    }

    g_target = output.device_methods[kEndSceneVtableIndex];
    g_reset_target = output.device_methods[kResetVtableIndex];

    const MH_STATUS init = MH_Initialize();
    if (init == MH_ERROR_ALREADY_INITIALIZED) {
        g_minhook_owned = false;
    } else if (init != MH_OK) {
        KIERO_DBG_MSG("tounel: MH_Initialize failed: %s", MH_StatusToString(init));
        return false;
    } else {
        g_minhook_owned = true;
    }

    const MH_STATUS created = MH_CreateHook(
        g_target,
        reinterpret_cast<LPVOID>(&hkEndScene),
        reinterpret_cast<LPVOID*>(&oEndScene));
    if (created != MH_OK) {
        KIERO_DBG_MSG("tounel: MH_CreateHook(EndScene) failed: %s", MH_StatusToString(created));
        if (g_minhook_owned) MH_Uninitialize();
        return false;
    }

    const MH_STATUS created_reset = MH_CreateHook(
        g_reset_target,
        reinterpret_cast<LPVOID>(&hkReset),
        reinterpret_cast<LPVOID*>(&oReset));
    if (created_reset != MH_OK) {
        KIERO_DBG_MSG("tounel: MH_CreateHook(Reset) failed: %s", MH_StatusToString(created_reset));
        MH_RemoveHook(g_target);
        if (g_minhook_owned) MH_Uninitialize();
        return false;
    }

    const MH_STATUS enabled = MH_EnableHook(MH_ALL_HOOKS);
    if (enabled != MH_OK) {
        KIERO_DBG_MSG("tounel: MH_EnableHook failed: %s", MH_StatusToString(enabled));
        MH_RemoveHook(g_reset_target);
        MH_RemoveHook(g_target);
        if (g_minhook_owned) MH_Uninitialize();
        return false;
    }
    return true;
}

DWORD WINAPI HookThreadProc(LPVOID)
{
    if (!WaitForD3D9(kD3D9WaitTimeoutMs)) {
        KIERO_DBG_MSG("tounel: timed out waiting for d3d9.dll");
        return 1;
    }
    if (!InstallEndSceneHook())  return 1;
    return 0;
}

} // namespace

    void StartHookThread()
    {
        if (g_hook_thread) return;
        
        g_hook_thread = CreateThread(nullptr, 0, &HookThreadProc, nullptr, 0, nullptr);
    }

    void RemoveHooks()
    {
        if (g_hook_thread) {
            if (WaitForSingleObject(g_hook_thread, kHookThreadJoinTimeoutMs) != WAIT_OBJECT_0) {
                KIERO_DBG_MSG("tounel: hook thread still busy at unload");
            }
            else {
                CloseHandle(g_hook_thread);
                g_hook_thread = nullptr;
            }
        }

        if (oEndScene) {
            MH_DisableHook(MH_ALL_HOOKS);

            tounel::menu::Shutdown();

            MH_RemoveHook(g_reset_target);
            MH_RemoveHook(g_target);
            if (g_minhook_owned) MH_Uninitialize();

            g_reset_target = nullptr;
            g_target = nullptr;
            oReset = nullptr;
            oEndScene = nullptr;
            g_minhook_owned = false;
        }
    } // namespace tounel
