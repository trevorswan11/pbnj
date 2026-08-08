#pragma once

#include <array>

namespace pbnj::ui::assets {

constexpr auto app_icon = std::to_array<unsigned char>({
#include "ui/assets/raw/app_icon.inc"
});

} // namespace pbnj::ui::assets
