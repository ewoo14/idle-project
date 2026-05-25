#pragma once

#include "CoreMinimal.h"

namespace IdleProject::UI::Theme
{
inline constexpr FLinearColor BgPrimary = FLinearColor(0x1A / 255.f, 0x1B / 255.f, 0x2E / 255.f, 1.f);
inline constexpr FLinearColor BgPanel = FLinearColor(0x25 / 255.f, 0x28 / 255.f, 0x45 / 255.f, 1.f);

inline constexpr FLinearColor TextPrimary = FLinearColor(0xF5 / 255.f, 0xF5 / 255.f, 0xF0 / 255.f, 1.f);
inline constexpr FLinearColor TextMuted = FLinearColor(0xA0 / 255.f, 0xA4 / 255.f, 0xB8 / 255.f, 1.f);

inline constexpr FLinearColor AccentGold = FLinearColor(0xF4 / 255.f, 0xC4 / 255.f, 0x4A / 255.f, 1.f);
inline constexpr FLinearColor AccentBlue = FLinearColor(0x6B / 255.f, 0xB7 / 255.f, 0xF0 / 255.f, 1.f);
inline constexpr FLinearColor AccentRed = FLinearColor(0xE5 / 255.f, 0x55 / 255.f, 0x6B / 255.f, 1.f);

inline constexpr FLinearColor ErrorCritical = FLinearColor(0xFF / 255.f, 0x4D / 255.f, 0x5E / 255.f, 1.f);
inline constexpr FLinearColor Warn = FLinearColor(0xF0 / 255.f, 0xA4 / 255.f, 0x3A / 255.f, 1.f);
inline constexpr FLinearColor Info = FLinearColor(0x5B / 255.f, 0xA7 / 255.f, 0xF0 / 255.f, 1.f);
inline constexpr FLinearColor Auth = FLinearColor(0xC0 / 255.f, 0x71 / 255.f, 0xF2 / 255.f, 1.f);

inline constexpr FLinearColor RarityCommon = FLinearColor(0xB0 / 255.f, 0xB5 / 255.f, 0xC0 / 255.f, 1.f);
inline constexpr FLinearColor RarityUncommon = FLinearColor(0x5B / 255.f, 0xC0 / 255.f, 0x7A / 255.f, 1.f);
inline constexpr FLinearColor RarityRare = FLinearColor(0x5B / 255.f, 0x8B / 255.f, 0xC0 / 255.f, 1.f);
inline constexpr FLinearColor RarityEpic = FLinearColor(0xA0 / 255.f, 0x5B / 255.f, 0xC0 / 255.f, 1.f);
inline constexpr FLinearColor RarityLegendary = FLinearColor(0xF0 / 255.f, 0xA0 / 255.f, 0x40 / 255.f, 1.f);
inline constexpr FLinearColor RarityMythicStart = FLinearColor(0xFF / 255.f, 0x7B / 255.f, 0x00 / 255.f, 1.f);
inline constexpr FLinearColor RarityMythicEnd = FLinearColor(0x00 / 255.f, 0xB4 / 255.f, 0xFF / 255.f, 1.f);
}
