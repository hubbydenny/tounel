#include "pch.hpp"
#include "menu.hpp"

#include "imgui.h"
#include "imgui_impl_dx9.h"
#include "imgui_impl_win32.h"

#include <atomic>
#include <cstdio>

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(
    HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

namespace {

std::atomic<bool> g_context_ready{false};
std::atomic<bool> g_device_ready{false};
std::atomic<IDirect3DDevice9*> g_device{nullptr};
std::atomic<ULONG> g_frames{0};
HWND g_hwnd = nullptr;
WNDPROC g_orig_wndproc = nullptr;

HMODULE g_module = nullptr;
bool g_visible = false;

ImGuiKey g_toggle_key = ImGuiKey_Insert;
ImGuiKey g_unload_key = ImGuiKey_End;

bool g_show_debug = true;
float g_fps_smooth = 0.0f;
ULONG g_last_frames = 0;
double g_fps_time = 0.0;

LRESULT CALLBACK WndProcProxy(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
    if (g_context_ready.load(std::memory_order_acquire)) {
        if (ImGui_ImplWin32_WndProcHandler(hwnd, msg, wparam, lparam)) {
            return 0;
        }
    }

    const WNDPROC next = g_orig_wndproc;
    return next ? CallWindowProcW(next, hwnd, msg, wparam, lparam)
                : DefWindowProcW(hwnd, msg, wparam, lparam);
}

HWND FindWindowForDevice(IDirect3DDevice9* device)
{
    D3DDEVICE_CREATION_PARAMETERS params = {};
    if (SUCCEEDED(device->GetCreationParameters(&params)) && params.hFocusWindow) {
        return params.hFocusWindow;
    }
    return GetForegroundWindow();
}

void UpdateFps(ULONG frame)
{
    const double now = ImGui::GetTime();
    if (now <= g_fps_time) return;

    const float fps = static_cast<float>(
        static_cast<double>(frame - g_last_frames) / (now - g_fps_time));

    g_last_frames = frame;
    g_fps_time = now;
    g_fps_smooth = g_fps_smooth <= 0.0f ? fps : (g_fps_smooth * 0.9f + fps * 0.1f);
}

void KeyRow(const char* label, ImGuiKey* key)
{ 
    ImGui::TextUnformatted(label);
    ImGui::SameLine(120.0f);
    ImGui::TextUnformatted(ImGui::GetKeyName(*key));

    if (ImGui::IsItemHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
        ImGui::OpenPopup("bind");
    }

    if (ImGui::BeginPopup("bind")) {
        ImGui::TextUnformatted("press a key");
        ImGui::Separator();
        for (int i = ImGuiKey_NamedKey_BEGIN; i < ImGuiKey_NamedKey_END; ++i) {
            const ImGuiKey candidate = static_cast<ImGuiKey>(i);
            const char* const name = ImGui::GetKeyName(candidate);
            if (!name || !ImGui::IsKeyPressed(candidate)) continue;

            *key = candidate;
            ImGui::CloseCurrentPopup();
            break;
        }
        ImGui::EndPopup();
    }
}

void DrawStatus(ULONG frame)
{
    ImGui::Text("FPS:       %.1f", g_fps_smooth);
    ImGui::Text("Frames:    %lu", frame);
    ImGui::Text("Device:    %p", g_device.load(std::memory_order_relaxed));
    ImGui::Text("HWND:      %p", static_cast<void*>(g_hwnd));
    ImGui::Text("ImGui:     %s", IMGUI_VERSION);
}

void DrawSettings()
{
    ImGui::Checkbox("Show debug", &g_show_debug);

    KeyRow("Toggle menu:", &g_toggle_key);
    KeyRow("Unload DLL: ", &g_unload_key);

    if (ImGui::Button("Unload"))
        FreeLibraryAndExitThread(g_module, 0);
}

void Draw(ULONG frame)
{
    if (ImGui::IsKeyPressed(g_toggle_key, false))
        g_visible = !g_visible;

    if (ImGui::IsKeyPressed(g_unload_key, false))
        FreeLibraryAndExitThread(g_module, 0);

    if (!g_visible) return;

    ImGui::SetNextWindowSize(ImVec2(380.0f, 0.0f), ImGuiCond_FirstUseEver);

    if (!ImGui::Begin("tounel")) {
        ImGui::End();
        return;
    }

    if (g_show_debug)
        DrawStatus(frame);

    if (ImGui::CollapsingHeader("Settings", ImGuiTreeNodeFlags_DefaultOpen))
        DrawSettings();

    ImGui::End();
}

} // namespace

namespace tounel {
namespace menu {

bool Initialize(IDirect3DDevice9* device)
{
    if (g_device_ready.load(std::memory_order_acquire)) return true;

    if (!g_context_ready.load(std::memory_order_acquire)) {
        g_module = GetModuleHandleW(L"tounel.dll");

        HWND hwnd = FindWindowForDevice(device);
        if (!hwnd) return false;

        IMGUI_CHECKVERSION();
        ImGui::CreateContext();

        ImGuiIO& io = ImGui::GetIO();
        io.IniFilename = nullptr;
        io.LogFilename = nullptr;
        io.ConfigFlags |= ImGuiConfigFlags_NoMouseCursorChange;

        ImGui::StyleColorsDark();

        if (!ImGui_ImplWin32_Init(hwnd)) {
            ImGui::DestroyContext();
            return false;
        }
        if (!ImGui_ImplDX9_Init(device)) {
            ImGui_ImplWin32_Shutdown();
            ImGui::DestroyContext();
            return false;
        }

        WNDPROC current = reinterpret_cast<WNDPROC>(GetWindowLongPtrW(hwnd, GWLP_WNDPROC));
        if (current == &WndProcProxy)
            current = g_orig_wndproc;
        if (current) {
            g_orig_wndproc = current;
            SetWindowLongPtrW(hwnd, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(&WndProcProxy));
        } else {
            KIERO_DBG_MSG("menu: no WndProc to chain");
        }

        g_hwnd = hwnd;
        g_frames.store(0, std::memory_order_relaxed);
        g_last_frames = 0;
        g_fps_time = ImGui::GetTime();
        g_context_ready.store(true, std::memory_order_release);
    }

    if (!ImGui_ImplDX9_CreateDeviceObjects())
        KIERO_DBG_MSG("menu: ImGui_ImplDX9_CreateDeviceObjects failed");

    g_device.store(device, std::memory_order_release);
    g_device_ready.store(true, std::memory_order_release);
    return true;
}

void Shutdown()
{
    if (!g_context_ready.exchange(false)) return;
    g_device_ready.store(false, std::memory_order_release);

    if (g_hwnd && g_orig_wndproc) {
        SetWindowLongPtrW(g_hwnd, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(g_orig_wndproc));
    }
    g_orig_wndproc = nullptr;
    g_hwnd = nullptr;

    ImGui_ImplDX9_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();

    g_device.store(nullptr, std::memory_order_release);
}

void RenderFrame()
{
    if (!g_device_ready.load(std::memory_order_acquire)) return;

    ImGui_ImplDX9_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();

    const ULONG frame = g_frames.fetch_add(1, std::memory_order_relaxed) + 1;
    UpdateFps(frame);
    Draw(frame);

    ImGui::Render();
    ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());
}

void InvalidateDevice()
{
    if (!g_device_ready.exchange(false)) return;
    ImGui_ImplDX9_InvalidateDeviceObjects();
    g_device.store(nullptr, std::memory_order_release);
}

void RecreateDevice()
{
    if (!g_context_ready.load(std::memory_order_acquire)) return;
    if (g_device_ready.load(std::memory_order_acquire)) return;

    if (!ImGui_ImplDX9_CreateDeviceObjects())
        KIERO_DBG_MSG("menu: ImGui_ImplDX9_CreateDeviceObjects failed");

    g_device_ready.store(true, std::memory_order_release);
}

} // namespace menu
} // namespace tounel
