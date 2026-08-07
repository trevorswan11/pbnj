#pragma once

#include <array>

namespace pbnj::ui::assets {

constexpr auto APP_ICON_PNG = std::to_array<unsigned char>({
#include "ui/assets/app_icon.inc"
});

} // namespace pbnj::ui::assets
