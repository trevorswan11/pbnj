#pragma once

#include <string>

#include <stdx/types.hh>

namespace pbnj::support {

struct github_config {
    std::string token;
    std::string default_owner;
    std::string default_repo;

    [[nodiscard]] auto operator==(const github_config&) const noexcept -> bool = default;
};

struct azure_config {
    std::string token;
    std::string organization;
    std::string project;
    std::string repository;

    [[nodiscard]] auto operator==(const azure_config&) const noexcept -> bool = default;
};

struct ai_config {
    std::string provider;
    std::string base_url;
    std::string api_key;
    std::string model;
    f32         temperature{0.2f};
    f32         confidence_threshold{0.80f};
    std::string custom_rules;

    [[nodiscard]] auto operator==(const ai_config&) const noexcept -> bool = default;
};

struct preferences_config {
    std::string theme{"dark"};
    std::string diff_view_mode{"split"};
    bool        show_whitespace{false};
    bool        auto_ai_review{false};
    u32         lines_context{3};

    [[nodiscard]] auto operator==(const preferences_config&) const noexcept -> bool = default;
};

struct app_config {
    github_config      github;
    azure_config       azure;
    ai_config          ai;
    preferences_config preferences;

    [[nodiscard]] auto operator==(const app_config&) const noexcept -> bool = default;
};

} // namespace pbnj::support
