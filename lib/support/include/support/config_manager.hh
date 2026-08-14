#pragma once

#include <filesystem>
#include <string>
#include <string_view>
#include <utility>

#include <stdx/types.hh>

#include "support/error.hh"

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
    std::string provider{"anthropic"};
    std::string base_url{"https://api.anthropic.com/v1/messages"};
    std::string api_key;
    std::string model{"claude-3-5-sonnet-20241022"};
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

class config_manager {
  public:
    config_manager() : path_{get_default_config_path()} {}
    explicit config_manager(std::filesystem::path config_path) : path_{std::move(config_path)} {}

    [[nodiscard]] auto get() const noexcept -> const app_config& { return config_; }
    [[nodiscard]] auto get_mut() noexcept -> app_config& { return config_; }
    auto               set(app_config config) noexcept -> void { config_ = std::move(config); }

    [[nodiscard]] auto get_config_path() const noexcept -> const std::filesystem::path& {
        return path_;
    }
    auto set_config_path(std::filesystem::path path) noexcept -> void { path_ = std::move(path); }

    [[nodiscard]] auto load() -> result<void> { return load_from(path_); }
    [[nodiscard]] auto load_from(const std::filesystem::path& path) -> result<void>;
    [[nodiscard]] auto load_from_string(std::string_view json_str) -> result<void>;

    [[nodiscard]] auto save() const -> result<void> { return save_to(path_); }
    [[nodiscard]] auto save_to(const std::filesystem::path& path) const -> result<void>;
    [[nodiscard]] auto to_json_string() const -> std::string;

    auto reset() noexcept -> void { config_ = app_config{}; }

    [[nodiscard]] static auto get_default_config_path() -> std::filesystem::path;

  private:
    app_config            config_;
    std::filesystem::path path_;
};

} // namespace pbnj::support
