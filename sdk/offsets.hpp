#pragma once
#include <cstddef>
#include <cstdint>

namespace base_entity {
    constexpr std::uint32_t m_iHealth = 0xD0;
    constexpr std::uint32_t m_iTeamNum = 0xD8;
    constexpr std::uint32_t m_vecOrigin = 0x428;
    constexpr std::uint32_t m_fFlags = 0x440;
    constexpr std::uint32_t m_lifeState = 0xCF;
    constexpr std::uint32_t m_nModelIndex = 0xCC;
    constexpr std::uint32_t m_hOwnerEntity = 0x734;
    constexpr std::uint32_t m_CollisionGroup = 0x444;
    constexpr std::uint32_t m_Collision = 0x220;
    constexpr std::uint32_t m_vecMins = m_Collision + 0x28;
    constexpr std::uint32_t m_vecMaxs = m_Collision + 0x34;
    constexpr std::uint32_t m_MoveType = 0x0F4; // .data:00000001805A61A8                 dq offset aMMovetype    ; "m_MoveType" .data:00000001805A61B0                 db 0F4h
}
namespace base_player {
    constexpr std::uint32_t local = 0x1208;
    constexpr std::uint32_t m_vecViewOffset = 0x13C;
    constexpr std::uint32_t m_vecVelocity = 0x148;
    constexpr std::uint32_t m_aimPunchAngle = local + 0x74;
    constexpr std::uint32_t m_hActiveWeapon = 0x11A0;
    constexpr std::uint32_t m_nTickBase = 0x16B0;
    constexpr std::uint32_t m_iShotsFired = 0x1A34;
    constexpr std::uint32_t m_ArmorValue = 0x1AA4;
    constexpr std::uint32_t m_bHasHelmet = 0x1A9C;
    constexpr std::uint32_t m_bIsScoped = 0x0;
    constexpr std::uint32_t m_iFOV = 0x1570;
    constexpr std::uint32_t m_iDefaultFOV = 0x157C;
    constexpr std::uint32_t m_iObserverMode = 0x15DC;
    constexpr std::uint32_t m_hObserverTarget = 0x15E0;
    constexpr std::uint32_t m_iAccount = 0x1A98;
    constexpr std::uint32_t m_flFlashDuration = 0x1A58;
    constexpr std::uint32_t m_flFlashMaxAlpha = 0x1A54;
    constexpr std::uint32_t m_vecPunchAngle = local + 0x74;
    constexpr std::uint32_t m_angEyeAngles = 0x1AA8;
}