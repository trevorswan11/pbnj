const std = @import("std");
const stdx = @import("../build.zig").stdx;

const Dependency = stdx.Dependency;

pub fn build(b: *std.Build, config: Dependency.Config) struct {
    imgui_upstream: *std.Build.Dependency,
    sokol_upstream: *std.Build.Dependency,
    artifact: *std.Build.Step.Compile,
} {
    const imgui_upstream = b.dependency("imgui", .{});
    const sokol_upstream = b.dependency("sokol", .{});
    const mod = b.createModule(.{
        .optimize = config.optimize,
        .target = config.target,
        .link_libc = true,
        .link_libcpp = true,
    });

    const file = b.addWriteFile("sokol.cpp",
        \\#include <imgui.h>
        \\
        \\#include <sokol_app.h>
        \\#include <sokol_gfx.h>
        \\#include <sokol_glue.h>
        \\#include <sokol_imgui.h>
        \\#include <sokol_log.h>
    );

    var sokol_flags: stdx.ArrayList([]const u8) = .fromSlice(b, &.{
        "-DSOKOL_APP_IMPL",
        "-DSOKOL_GFX_IMPL",
        "-DSOKOL_GLUE_IMPL",
        "-DSOKOL_IMGUI_IMPL",
        "-DSOKOL_LOG_IMPL",
        "-DSOKOL_NO_ENTRY",
    });

    const is_linux = config.target.result.os.tag == .linux;
    const is_windows = config.target.result.os.tag == .windows;
    const is_darwin = config.target.result.os.tag.isDarwin();
    Dependency.addFrameworkSearchPaths(mod, config.target);
    if (is_darwin) {
        sokol_flags.append("-DSOKOL_METAL");
        mod.linkFramework("Cocoa", .{});
        mod.linkFramework("IOKit", .{});
        mod.linkFramework("Metal", .{});
        mod.linkFramework("MetalKit", .{});
        mod.linkFramework("QuartzCore", .{});
        mod.linkFramework("AVFoundation", .{});
        mod.linkFramework("CoreMedia", .{});
        mod.linkFramework("CoreVideo", .{});
    } else if (is_windows) {
        sokol_flags.append("-DSOKOL_D3D11");
        mod.linkSystemLibrary("d3d11", .{});
        mod.linkSystemLibrary("dxgi", .{});
        mod.linkSystemLibrary("kernel32", .{});
        mod.linkSystemLibrary("user32", .{});
        mod.linkSystemLibrary("gdi32", .{});
        mod.linkSystemLibrary("ole32", .{});
    } else if (is_linux) {
        sokol_flags.append("-DSOKOL_GLCORE");
        mod.linkSystemLibrary("GL", .{});
        mod.linkSystemLibrary("X11", .{});
        mod.linkSystemLibrary("Xi", .{});
        mod.linkSystemLibrary("Xcursor", .{});
        mod.linkSystemLibrary("asound", .{});
    }

    const sokol_include = sokol_upstream.path(".");
    const sokol_util = sokol_upstream.path("util");
    mod.addIncludePath(sokol_include);
    mod.addCSourceFile(.{
        .file = file.getDirectory().path(b, "sokol.cpp"),
        .flags = sokol_flags.wrapped.items,
        .language = if (is_darwin) .objective_cpp else null,
    });
    mod.addIncludePath(sokol_util);

    const imgui_root = imgui_upstream.path(".");
    const imgui_misc_cpp = imgui_upstream.path("misc/cpp");
    mod.addIncludePath(imgui_root);
    mod.addCSourceFiles(.{
        .root = imgui_root,
        .files = &.{
            "imgui.cpp",
            "imgui_draw.cpp",
            "imgui_widgets.cpp",
            "imgui_tables.cpp",
            "imgui_demo.cpp",
            "misc/cpp/imgui_stdlib.cpp",
        },
    });
    mod.addIncludePath(imgui_upstream.path("backends"));
    mod.addIncludePath(imgui_misc_cpp);

    const lib = b.addLibrary(.{
        .name = "sokol",
        .root_module = mod,
    });
    lib.installHeadersDirectory(imgui_root, "", .{});
    lib.installHeadersDirectory(sokol_include, "", .{});
    lib.installHeadersDirectory(imgui_misc_cpp, "", .{});
    lib.installHeadersDirectory(sokol_util, "", .{});

    return .{
        .imgui_upstream = imgui_upstream,
        .sokol_upstream = sokol_upstream,
        .artifact = lib,
    };
}
