#include <stdx/profiler.hh>
#include <stdx/types.hh>

#include "ui/core/application.hh"

auto main(i32 /* argc */, char** argv) -> i32 {
    stdx::profiler        p{argv[0]};
    pbnj::ui::application app;
    app.launch();
}
