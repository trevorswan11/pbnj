#include "ui/pages/search.hh"

#include <imgui.h>
#include <stdx/profiler.hh>
#include <stdx/utility.hh>

#include "ui/core/component.hh"
#include "ui/core/context.hh"

namespace pbnj::ui::pages {

auto search::on_mount(context& ctx) -> void { ctx.log.info("Mounted search page"); }

auto search::on_unmount(context& ctx) -> void { ctx.log.info("Unmounted search page"); }

auto search::render(context& ctx) -> void {
    PROFILE_FUNCTION();

    if (ctx.search_input.empty()) {
        ImGui::TextDisabled("Type something in the top bar to search...");
    } else {
        ImGui::Text("Results for \"%s\":", ctx.search_input.c_str());
    }
}

} // namespace pbnj::ui::pages
