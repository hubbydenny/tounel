#pragma once

#include <cstddef>

namespace tounel {
namespace menu {

bool Initialize(IDirect3DDevice9* device);
void Shutdown();
void RenderFrame();
void InvalidateDevice();
void RecreateDevice();

} // namespace menu
} // namespace tounel