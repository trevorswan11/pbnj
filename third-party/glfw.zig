const std = @import("std");
const glfw = @import("sources/glfw.zig");
const stdx = @import("../build.zig").stdx;

const Dependency = stdx.Dependency;

pub fn build(b: *std.Build, config: Dependency.Config) Dependency {
    const upstream = b.dependency("glfw", .{});
    const target = config.target;
    const mod = b.createModule(.{
        .target = target,
        .optimize = config.optimize,
        .link_libc = true,
    });

    const src = upstream.path("src");
    const include = upstream.path("include");

    mod.addIncludePath(include);
    if (b.lazyDependency("vulkan_headers", .{})) |dep| {
        mod.addIncludePath(dep.path("include"));
    }
    if (target.result.os.tag == .linux) {
        if (b.lazyDependency("x11_headers", .{})) |dep| {
            mod.addSystemIncludePath(dep.path(""));
        }
        if (b.lazyDependency("wayland_headers", .{})) |dep| {
            mod.addSystemIncludePath(dep.path("wayland"));
            mod.addSystemIncludePath(dep.path("wayland-protocols"));
        }
    }

    if (target.result.os.tag.isDarwin()) {
        // This must be defined for macOS 13.3 and older.
        mod.addCMacro("__kernel_ptr_semantics", "");

        if (b.lazyDependency("xcode_frameworks", .{})) |dep| {
            mod.addSystemFrameworkPath(dep.path("Frameworks"));
            mod.addSystemIncludePath(dep.path("include"));
            mod.addLibraryPath(dep.path("lib"));
        }
    }

    mod.addCSourceFiles(.{
        .root = src,
        .files = &glfw.base_sources,
    });

    switch (target.result.os.tag) {
        .windows => {
            mod.linkSystemLibrary("gdi32", .{});
            mod.linkSystemLibrary("user32", .{});
            mod.linkSystemLibrary("shell32", .{});

            if (use_opengl) mod.linkSystemLibrary("opengl32", .{});
            if (use_gles) mod.linkSystemLibrary("GLESv3", .{});

            mod.addCMacro("_GLFW_WIN32", "1");
            mod.addCSourceFiles(.{
                .root = src,
                .files = &glfw.windows_sources,
            });
        },
        .macos => {
            // Transitive dependencies
            mod.linkFramework("CFNetwork", .{});
            mod.linkFramework("ApplicationServices", .{});
            mod.linkFramework("ColorSync", .{});
            mod.linkFramework("CoreText", .{});
            mod.linkFramework("ImageIO", .{});

            // Direct dependencies
            mod.linkSystemLibrary("objc", .{});
            mod.linkFramework("IOKit", .{});
            mod.linkFramework("CoreFoundation", .{});
            mod.linkFramework("AppKit", .{});
            mod.linkFramework("CoreServices", .{});
            mod.linkFramework("CoreGraphics", .{});
            mod.linkFramework("Foundation", .{});
            mod.linkFramework("QuartzCore", .{});

            if (use_metal) mod.linkFramework("Metal", .{});
            if (use_opengl) mod.linkFramework("OpenGL", .{});

            mod.addCMacro("_GLFW_COCOA", "1");
            mod.addCSourceFiles(.{
                .root = src,
                .files = &glfw.macos_sources,
            });
        },

        // Everything that isn't windows or mac is linux
        else => {
            mod.addCSourceFiles(.{
                .root = src,
                .files = &glfw.linux_sources,
            });

            if (use_x11) {
                mod.addCMacro("_GLFW_X11", "1");
                mod.addCSourceFiles(.{
                    .root = src,
                    .files = &glfw.linux_x11_sources,
                });
            }

            if (use_wl) {
                mod.addCMacro("_GLFW_WAYLAND", "1");
                mod.addCSourceFiles(.{
                    .root = src,
                    .files = &glfw.linux_wl_sources,
                    .flags = &.{
                        "-Wno-implicit-function-declaration",
                    },
                });
            }
        },
    }

    const wrapper_header = b.addWriteFile("glfw.hh",
        \\#pragma once
        \\
        \\// IWYU pragma: begin_exports
        \\
        \\#ifndef GLFW_INCLUDE_NONE
        \\#define GLFW_INCLUDE_NONE
        \\#endif
        \\#include <GLFW/glfw3.h>
        \\#include <GLFW/glfw3native.h>
        \\
        \\// IWYU pragma: end_exports
        \\
    );

    const lib: *std.Build.Step.Compile = b.addLibrary(.{
        .name = "glfw",
        .root_module = mod,
    });
    lib.installHeadersDirectory(include.path(b, "GLFW"), "GLFW", .{});
    lib.installLibraryHeaders(lib);
    lib.installHeadersDirectory(wrapper_header.getDirectory(), "", .{
        .include_extensions = &.{ "h", "hh" },
    });

    return .{
        .upstream = upstream,
        .artifact = lib,
    };
}

// TODO(tcs): Make these real options
const use_x11 = true; // Only useful on Linux
const use_wl = false; // Only useful on Linux

const use_opengl = false; // Deprecated on MacOS
const use_gles = false; // Not supported on MacOS
const use_metal = false; // Only supported on MacOS
