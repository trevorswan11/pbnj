const std = @import("std");
const glfw = @import("glfw.zig");
const stdx = @import("../build.zig").stdx;

pub fn build(b: *std.Build, glfw_dep: stdx.Dependency, config: stdx.Dependency.Config) struct {
    imgui_upstream: *std.Build.Dependency,
    vulkan_headers_upstream: *std.Build.Dependency,
    volk_upstream: *std.Build.Dependency,
    vma_upstream: *std.Build.Dependency,
    artifact: *std.Build.Step.Compile,
} {
    const imgui_upstream = b.dependency("imgui", .{});
    const vulkan_headers_upstream = b.dependency("vulkan_headers", .{});
    const volk_upstream = b.dependency("volk", .{});
    const vma_upstream = b.dependency("vma", .{});

    const mod = b.createModule(.{
        .optimize = config.optimize,
        .target = config.target,
        .link_libc = true,
        .link_libcpp = true,
    });
    mod.linkLibrary(glfw_dep.artifact);

    const is_linux = config.target.result.os.tag == .linux;
    const is_windows = config.target.result.os.tag == .windows;
    const is_darwin = config.target.result.os.tag.isDarwin();
    stdx.addFrameworkSearchPaths(mod, config.target);

    if (is_darwin) {
        mod.linkFramework("Cocoa", .{});
        mod.linkFramework("IOKit", .{});
        mod.linkFramework("CoreFoundation", .{});
        mod.linkFramework("CoreVideo", .{});
        mod.linkFramework("QuartzCore", .{});
    } else if (is_windows) {
        mod.linkSystemLibrary("gdi32", .{});
        mod.linkSystemLibrary("user32", .{});
        mod.linkSystemLibrary("shell32", .{});
        mod.linkSystemLibrary("dwmapi", .{});
    } else if (is_linux) {
        mod.linkSystemLibrary("X11", .{});
        mod.linkSystemLibrary("Xi", .{});
        mod.linkSystemLibrary("Xcursor", .{});
        mod.linkSystemLibrary("Xrandr", .{});
        mod.linkSystemLibrary("Xxf86vm", .{});
        mod.linkSystemLibrary("dl", .{});
        mod.linkSystemLibrary("pthread", .{});
        mod.linkSystemLibrary("m", .{});
    }

    const vk_include = vulkan_headers_upstream.path("include");
    const volk_include = volk_upstream.path(".");
    const vma_include = vma_upstream.path("include");
    const glfw_include = glfw_dep.upstream.path("include");
    const imgui_root = imgui_upstream.path(".");
    const imgui_backends = imgui_upstream.path("backends");
    const imgui_misc_cpp = imgui_upstream.path("misc/cpp");

    mod.addIncludePath(vk_include);
    mod.addIncludePath(volk_include);
    mod.addIncludePath(vma_include);
    mod.addIncludePath(glfw_include);
    mod.addIncludePath(imgui_root);
    mod.addIncludePath(imgui_backends);
    mod.addIncludePath(imgui_misc_cpp);

    const vma_cc = b.addWriteFile("vma.cc",
        \\#define VMA_IMPLEMENTATION
        \\#define VMA_STATIC_VULKAN_FUNCTIONS 0
        \\#define VMA_DYNAMIC_VULKAN_FUNCTIONS 1
        \\#define VK_NO_PROTOTYPES
        \\#include <volk.h>
        \\#include <vk_mem_alloc.h>
        \\
    );

    // Volk C implementation
    mod.addCSourceFile(.{
        .file = volk_upstream.path("volk.c"),
        .flags = &.{"-DVK_NO_PROTOTYPES"},
    });

    // VMA C++ implementation
    mod.addCSourceFile(.{
        .file = vma_cc.getDirectory().path(b, "vma.cc"),
        .flags = &.{
            "-DVK_NO_PROTOTYPES",
            "-Wno-nullability-completeness",
            "-Wno-nullability-extension",
        },
    });

    // ImGui C++ files
    mod.addCSourceFiles(.{
        .root = imgui_root,
        .files = &.{
            "imgui.cpp",
            "imgui_draw.cpp",
            "imgui_widgets.cpp",
            "imgui_tables.cpp",
            "imgui_demo.cpp",
            "misc/cpp/imgui_stdlib.cpp",
            "backends/imgui_impl_glfw.cpp",
            "backends/imgui_impl_vulkan.cpp",
        },
        .flags = &.{
            "-DIMGUI_IMPL_VULKAN_USE_VOLK",
            "-DVK_NO_PROTOTYPES",
        },
    });

    const wrapper_header = b.addWriteFile("imgui.hh",
        \\#pragma once
        \\
        \\// IWYU pragma: begin_exports
        \\
        \\#ifndef VK_NO_PROTOTYPES
        \\#define VK_NO_PROTOTYPES
        \\#endif
        \\#include <volk.h>
        \\#ifndef GLFW_INCLUDE_NONE
        \\#define GLFW_INCLUDE_NONE
        \\#endif
        \\#include <GLFW/glfw3.h>
        \\#include <imgui.h>
        \\#include <imgui_impl_glfw.h>
        \\#include <imgui_impl_vulkan.h>
        \\#include <misc/cpp/imgui_stdlib.h>
        \\
        \\// IWYU pragma: end_exports
        \\
    );

    const lib = b.addLibrary(.{
        .name = "vulkan",
        .root_module = mod,
    });
    lib.installLibraryHeaders(glfw_dep.artifact);
    lib.installHeadersDirectory(vk_include, "", .{});
    lib.installHeadersDirectory(volk_include, "", .{ .include_extensions = &.{"h"} });
    lib.installHeadersDirectory(vma_include, "", .{ .include_extensions = &.{ "h", "hpp" } });
    lib.installHeadersDirectory(glfw_include, "", .{});
    lib.installHeadersDirectory(imgui_root, "", .{});
    lib.installHeadersDirectory(imgui_backends, "", .{});
    lib.installHeadersDirectory(imgui_misc_cpp, "", .{});
    lib.installHeadersDirectory(wrapper_header.getDirectory(), "", .{
        .include_extensions = &.{ "h", "hh" },
    });

    return .{
        .imgui_upstream = imgui_upstream,
        .vulkan_headers_upstream = vulkan_headers_upstream,
        .volk_upstream = volk_upstream,
        .vma_upstream = vma_upstream,
        .artifact = lib,
    };
}
