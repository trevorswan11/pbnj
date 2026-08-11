#pragma once

#include <sokol.h>
#include <stdx/option.hh>
#include <stdx/utility.hh>

template <> struct stdx::nullable<sg_image> {
    [[nodiscard]] static constexpr auto invalid() noexcept -> sg_image {
        return sg_image{SG_INVALID_ID};
    }

    [[nodiscard]] static constexpr auto is_valid(sg_image img) noexcept -> bool {
        return img.id != SG_INVALID_ID;
    }
};

template <> struct stdx::nullable<sg_view> {
    [[nodiscard]] static constexpr auto invalid() noexcept -> sg_view {
        return sg_view{SG_INVALID_ID};
    }

    [[nodiscard]] static constexpr auto is_valid(sg_view view) noexcept -> bool {
        return view.id != SG_INVALID_ID;
    }
};

template <> struct stdx::nullable<ImTextureID> {
    [[nodiscard]] static constexpr auto invalid() noexcept -> ImTextureID {
        return ImTextureID{ImTextureID_Invalid};
    }

    [[nodiscard]] static constexpr auto is_valid(ImTextureID id) noexcept -> bool {
        return id != ImTextureID_Invalid;
    }
};

namespace pbnj::ui::assets {

struct texture {
    texture() = default;
    ~texture() {
        if (sg_isvalid()) {
            if (view) { sg_destroy_view(view.take()); }
            if (image) { sg_destroy_image(image.take()); }
        }
        imgui_id.reset();
    }
    MAKE_MOVE_ONLY(texture);

    stdx::option<sg_image>    image;
    stdx::option<sg_view>     view;
    stdx::option<ImTextureID> imgui_id;
};

} // namespace pbnj::ui::assets
