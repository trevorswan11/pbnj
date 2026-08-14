#pragma once

#include <stdx/result.hh>
#include <stdx/types.hh>

namespace pbnj {

enum class error : u8 {
    SVG_PARSE_FAILED,
    GPU_DEVICE_NOT_FOUND,
    GPU_TEXTURE_ALLOC_FAILED,
    GPU_TRANSFER_BUFFER_FAILED,
    GPU_SAMPLER_CREATE_FAILED,
    INVALID_IMGUI_TEXTURE,
};

template <typename T> using result = stdx::result<T, error>;

} // namespace pbnj
