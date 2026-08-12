#pragma once

#include <sokol.h>
#include <stdx/option.hh>
#include <stdx/utility.hh>

#include "ui/core/compact_types.hh" // IWYU pragma: keep

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
    texture(const texture&)                    = delete;
    auto operator=(const texture&) -> texture& = delete;

    texture(texture&& other) noexcept
        : image{other.image.take()}, view{other.view.take()}, imgui_id{other.imgui_id.take()} {}

    auto operator=(texture&& other) noexcept -> texture& {
        if (this != &other) {
            if (sg_isvalid()) {
                if (view) { sg_destroy_view(view.take()); }
                if (image) { sg_destroy_image(image.take()); }
            }
            image    = other.image.take();
            view     = other.view.take();
            imgui_id = other.imgui_id.take();
        }
        return *this;
    }

    stdx::option<sg_image>    image;
    stdx::option<sg_view>     view;
    stdx::option<ImTextureID> imgui_id;
};

} // namespace pbnj::ui::assets
