const std = @import("std");
const stdx = @import("../build.zig").stdx;

const Dependency = stdx.Dependency;

pub fn build(b: *std.Build, config: Dependency.Config) Dependency {
    const upstream = b.dependency("nanosvg", .{});
    const mod = b.createModule(.{
        .optimize = config.optimize,
        .target = config.target,
        .link_libc = true,
    });

    const file = b.addWriteFile("nanosvg.cc",
        \\#include <nanosvg.h>
        \\#include <nanosvgrast.h>
    );
    const src = upstream.path("src");
    mod.addIncludePath(src);
    mod.addCSourceFile(.{
        .file = file.getDirectory().path(b, "nanosvg.cc"),
        .flags = &.{ "-DNANOSVG_IMPLEMENTATION", "-DNANOSVGRAST_IMPLEMENTATION" },
    });

    const lib = b.addLibrary(.{
        .name = "nanosvg",
        .root_module = mod,
    });
    lib.installHeadersDirectory(src, "", .{});
    return .{ .upstream = upstream, .artifact = lib };
}
