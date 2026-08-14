#pragma once

#include <imgui.hh>
#include <stdx/option.hh>

template <> struct stdx::nullable<ImTextureID> {
    [[nodiscard]] static constexpr auto invalid() noexcept -> ImTextureID {
        return ImTextureID{ImTextureID_Invalid};
    }

    [[nodiscard]] static constexpr auto is_valid(ImTextureID id) noexcept -> bool {
        return id != ImTextureID_Invalid;
    }
};
