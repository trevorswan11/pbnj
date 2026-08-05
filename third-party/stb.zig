const std = @import("std");
const stdx = @import("../build.zig").stdx;

const Dependency = stdx.Dependency;

pub fn build(b: *std.Build, config: Dependency.Config) Dependency {
    const upstream = b.dependency("stb", .{});
    const mod = b.addModule("stb", .{
        .optimize = config.optimize,
        .target = config.target,
        .link_libc = true,
    });

    const file = b.addWriteFile("stb.c", "#include <stb_image.h>");
    const include = upstream.path(".");
    mod.addIncludePath(include);
    mod.addCSourceFile(.{
        .file = file.getDirectory().path(b, "stb.c"),
        .flags = &.{"-DSTB_IMAGE_IMPLEMENTATION"},
    });

    const lib = b.addLibrary(.{
        .name = "stb",
        .root_module = mod,
    });
    lib.installHeadersDirectory(include, "", .{});
    return .{ .upstream = upstream, .artifact = lib };
}
