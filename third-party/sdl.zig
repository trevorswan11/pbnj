const std = @import("std");
const stdx = @import("../build.zig").stdx;

const Dependency = stdx.Dependency;

pub fn build(b: *std.Build, config: Dependency.Config) struct {
    imgui_upstream: *std.Build.Dependency,
    sdl_upstream: *std.Build.Dependency,
    artifact: *std.Build.Step.Compile,
} {
    const imgui_upstream = b.dependency("imgui", .{});
    const sdl_upstream = b.dependency("sdl", .{
        .target = config.target,
        .optimize = config.optimize,
    });
    const sdl_artifact = sdl_upstream.artifact("SDL3");
    const sdl_raw_upstream = sdl_upstream.builder.dependency("sdl", .{});
    const sdl_include = sdl_raw_upstream.path("include");

    const mod = b.createModule(.{
        .optimize = config.optimize,
        .target = config.target,
        .link_libc = true,
        .link_libcpp = true,
    });
    mod.linkLibrary(sdl_artifact);

    const is_linux = config.target.result.os.tag == .linux;
    const is_windows = config.target.result.os.tag == .windows;
    const is_darwin = config.target.result.os.tag.isDarwin();
    Dependency.addFrameworkSearchPaths(mod, config.target);

    if (is_darwin) {
        mod.linkFramework("Cocoa", .{});
        mod.linkFramework("IOKit", .{});
        mod.linkFramework("Metal", .{});
        mod.linkFramework("QuartzCore", .{});
    } else if (is_windows) {
        mod.linkSystemLibrary("d3d12", .{});
        mod.linkSystemLibrary("dxgi", .{});
        mod.linkSystemLibrary("kernel32", .{});
        mod.linkSystemLibrary("user32", .{});
        mod.linkSystemLibrary("gdi32", .{});
        mod.linkSystemLibrary("ole32", .{});
        mod.linkSystemLibrary("dwmapi", .{});
    } else if (is_linux) {
        mod.linkSystemLibrary("X11", .{});
        mod.linkSystemLibrary("Xi", .{});
        mod.linkSystemLibrary("Xcursor", .{});
    }

    const imgui_root = imgui_upstream.path(".");
    const imgui_backends = imgui_upstream.path("backends");
    const imgui_misc_cpp = imgui_upstream.path("misc/cpp");

    mod.addIncludePath(sdl_include);
    mod.addIncludePath(imgui_root);
    mod.addIncludePath(imgui_backends);
    mod.addIncludePath(imgui_misc_cpp);

    mod.addCSourceFiles(.{
        .root = imgui_root,
        .files = &.{
            "imgui.cpp",
            "imgui_draw.cpp",
            "imgui_widgets.cpp",
            "imgui_tables.cpp",
            "imgui_demo.cpp",
            "misc/cpp/imgui_stdlib.cpp",
            "backends/imgui_impl_sdl3.cpp",
            "backends/imgui_impl_sdlgpu3.cpp",
        },
    });

    const wrapper_header = b.addWriteFile("imgui.hh",
        \\#pragma once
        \\
        \\// IWYU pragma: begin_exports
        \\
        \\#include <imgui.h>
        \\#include <imgui_impl_sdl3.h>
        \\#include <imgui_impl_sdlgpu3.h>
        \\#include <misc/cpp/imgui_stdlib.h>
        \\
        \\// IWYU pragma: end_exports
    );

    const lib = b.addLibrary(.{
        .name = "sdl",
        .root_module = mod,
    });
    lib.installHeadersDirectory(sdl_raw_upstream.path("include"), "", .{});
    lib.installHeadersDirectory(imgui_root, "", .{});
    lib.installHeadersDirectory(imgui_backends, "", .{});
    lib.installHeadersDirectory(imgui_misc_cpp, "", .{});
    lib.installHeadersDirectory(wrapper_header.getDirectory(), "", .{
        .include_extensions = &.{ "h", "hh" },
    });

    return .{
        .imgui_upstream = imgui_upstream,
        .sdl_upstream = sdl_upstream,
        .artifact = lib,
    };
}
