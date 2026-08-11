#pragma once

#include <array>

namespace pbnj::ui::assets {

constexpr auto chevron_left = std::to_array<char>({
#include "ui/assets/raw/chevron-left.inc"
});

constexpr auto chevron_right = std::to_array<char>({
#include "ui/assets/raw/chevron-right.inc"
});

} // namespace pbnj::ui::assets
