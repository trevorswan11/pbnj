#pragma once

#include <string>

#include <stdx/utility.hh>

#include "support/logger.hh"
#include "ui/assets/texture_cache.hh"
#include "ui/core/router.hh"
#include "ui/theme/fonts.hh"
#include "ui/theme/style.hh"

namespace pbnj::ui {

struct context {
    context()  = default;
    ~context() = default;
    MAKE_PINNED(context);

    theme::style_manager styles;
    theme::font_manager  fonts;
    logger               log;

    core::router          router;
    assets::texture_cache textures;
    std::string           search_input;
};

} // namespace pbnj::ui
