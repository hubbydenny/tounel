#pragma once

#include <cstddef>

inline constexpr std::size_t kEndSceneVtableIndex = 42;
inline constexpr std::size_t kResetVtableIndex = 16;
inline constexpr DWORD kD3D9WaitTimeoutMs = 120'000;
inline constexpr DWORD kD3D9PollIntervalMs = 50;
inline constexpr DWORD kHookThreadJoinTimeoutMs = 2000;
void StartHookThread();
void RemoveHooks();

namespace hooks {

};