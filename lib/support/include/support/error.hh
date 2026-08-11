#pragma once

#include <stdx/result.hh>
#include <stdx/types.hh>

namespace pbnj {

enum class error : u8 {
    SVG_PARSE_FAILED,
    SOKOL_IMG_ALLOC_FAILED,
    SOKOL_IMG_VIEW_ALLOC_FAILED,
    INVALID_IMGUI_TEXTURE,
    SOKOL_SAMPLER_CREATE_FAILED,
};

template <typename T> using result = stdx::result<T, error>;

} // namespace pbnj
