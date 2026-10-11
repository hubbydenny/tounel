#include "pch.hpp"
#include <Windows.h>
#include <cstdint>
#include <type_traits>

template <typename T>
bool Read(HANDLE process, std::uintptr_t address, T& value)
{
    static_assert(std::is_trivially_copyable_v<T>);
    std::size_t bytesRead = 0;
    return ReadProcessMemory(process,reinterpret_cast<LPCVOID>(address), &value, sizeof(T), &bytesRead) != 0 && bytesRead == sizeof(T);
}
