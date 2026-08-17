#include "ui/core/frame.hh"

#include <imgui.hh>
#include <stdx/types.hh>

#include "ui/core/vk_context.hh"

namespace pbnj::ui {

frame::frame(vk_context& vk_ctx, GLFWwindow* window, f64 dt, ImVec4 clear_color) noexcept
    : vk_ctx_{vk_ctx}, window_{window}, dt_{dt}, clear_color_{clear_color} {
    is_active_ = vk_ctx_.begin_frame(window_, clear_color_);
    if (is_active_) {
        ImGui_ImplVulkan_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
    }
}

frame::~frame() {
    if (is_active_) {
        ImGui::Render();
        vk_ctx_.end_frame(window_, ImGui::GetDrawData());
    }
}

} // namespace pbnj::ui
