#pragma once

#include <array>

namespace pbnj::ui::assets {

constinit inline auto inter_regular_18pt = std::to_array<unsigned char>({
#include "ui/assets/raw/inter_regular-18pt.inc"
});

constinit inline auto inter_bold_18pt = std::to_array<unsigned char>({
#include "ui/assets/raw/inter_bold-18pt.inc"
});

constinit inline auto inter_bold_24pt = std::to_array<unsigned char>({
#include "ui/assets/raw/inter_bold-24pt.inc"
});

constinit inline auto inter_bold_28pt = std::to_array<unsigned char>({
#include "ui/assets/raw/inter_bold-28pt.inc"
});

constinit inline auto inter_italic_18pt = std::to_array<unsigned char>({
#include "ui/assets/raw/inter_italic-18pt.inc"
});

} // namespace pbnj::ui::assets
