#pragma once
#include <Windows.h>
#include "pch.hpp"

template <typename T>
static T* GetInterface(const char* library, const char* name)
{
    HMODULE handle = GetModuleHandleA(library);
    if (!handle) return nullptr;

    using CreateInterfaceFn = void* (__cdecl*)(const char*, const char*);
    auto createInterface = reinterpret_cast<CreateInterfaceFn>(GetProcAddress(handle, "CreateInterface"));

    if (!createInterface) return nullptr;
    return static_cast<T*>(createInterface(name, nullptr));
}

static void* ClientMode = nullptr;
static void* Client = nullptr;
