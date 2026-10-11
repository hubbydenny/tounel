#pragma once

#include <Windows.h>
#include <cstddef>
#include <cstdlib>
#include <vector>

namespace memory {

struct SigByte {
    unsigned char value;
    bool          wildcard;
};

inline std::vector<SigByte> ParseSignature(const char* signature)
{
    std::vector<SigByte> bytes;
    if (!signature) return bytes;

    const char* p = signature;
    while (*p) {
        while (*p == ' ') ++p;
        if (!*p) break;

        if (*p == '?') {
            bytes.push_back({ 0, true });
            ++p;
            if (*p == '?') ++p;
        } else {
            const char hex[3] = { p[0], p[1], '\0' };
            bytes.push_back({ static_cast<unsigned char>(std::strtoul(hex, nullptr, 16)), false });
            p += 2;
        }

        while (*p && *p != ' ') ++p;
    }
    return bytes;
}

inline char* scan(const char* ModuleName, const char* Signature)
{
    const HMODULE HMod = GetModuleHandleA(ModuleName);
    if (!HMod) return nullptr;

    char* Base = reinterpret_cast<char*>(HMod);
    const auto* Dos = reinterpret_cast<PIMAGE_DOS_HEADER>(Base);
    if (Dos->e_magic != IMAGE_DOS_SIGNATURE) return nullptr;

    const auto* Nt = reinterpret_cast<PIMAGE_NT_HEADERS>(Base + Dos->e_lfanew);
    if (Nt->Signature != IMAGE_NT_SIGNATURE) return nullptr;

    const std::vector<SigByte> pattern = ParseSignature(Signature);
    if (pattern.empty()) return nullptr;

    const IMAGE_SECTION_HEADER* const sections = IMAGE_FIRST_SECTION(Nt);

    for (WORD i = 0; i < Nt->FileHeader.NumberOfSections; ++i) {
        const IMAGE_SECTION_HEADER& section = sections[i];

        if (!(section.Characteristics & IMAGE_SCN_MEM_EXECUTE)) continue;

        const std::size_t size = section.Misc.VirtualSize;
        if (size < pattern.size()) continue;

        char* const start = Base + section.VirtualAddress;

        for (std::size_t offset = 0; offset <= size - pattern.size(); ++offset) {
            bool matched = true;

            for (std::size_t k = 0; k < pattern.size(); ++k) {
                if (pattern[k].wildcard) continue;
                if (start[offset + k] != static_cast<char>(pattern[k].value)) {
                    matched = false;
                    break;
                }
            }

            if (matched) return start + offset;
        }
    }
    return nullptr;
}

}
