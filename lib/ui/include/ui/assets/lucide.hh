#pragma once

#include <array>

namespace pbnj::ui::assets {

constexpr auto chevron_left = std::to_array<char>({
#include "ui/assets/raw/chevron-left.inc"
});

constexpr auto chevron_right = std::to_array<char>({
#include "ui/assets/raw/chevron-right.inc"
});

constexpr auto close = std::to_array<char>({
#include "ui/assets/raw/close.inc"
});

constexpr auto home = std::to_array<char>({
#include "ui/assets/raw/home.inc"
});

constexpr auto menu = std::to_array<char>({
#include "ui/assets/raw/menu.inc"
});

constexpr auto pause = std::to_array<char>({
#include "ui/assets/raw/pause.inc"
});

constexpr auto play = std::to_array<char>({
#include "ui/assets/raw/play.inc"
});

constexpr auto search = std::to_array<char>({
#include "ui/assets/raw/search.inc"
});

constexpr auto skip_back = std::to_array<char>({
#include "ui/assets/raw/skip-back.inc"
});

constexpr auto skip_forward = std::to_array<char>({
#include "ui/assets/raw/skip-forward.inc"
});

constexpr auto user_round = std::to_array<char>({
#include "ui/assets/raw/user-round.inc"
});

constexpr auto volume_0 = std::to_array<char>({
#include "ui/assets/raw/volume-0.inc"
});

constexpr auto volume_1 = std::to_array<char>({
#include "ui/assets/raw/volume-1.inc"
});

constexpr auto volume_2 = std::to_array<char>({
#include "ui/assets/raw/volume-2.inc"
});

constexpr auto volume_x = std::to_array<char>({
#include "ui/assets/raw/volume-x.inc"
});

} // namespace pbnj::ui::assets
