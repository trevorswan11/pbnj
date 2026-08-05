const std = @import("std");
const stdx = @import("../build.zig").stdx;

const Dependency = stdx.Dependency;

pub fn build(b: *std.Build, config: Dependency.Config) Dependency {
    const upstream = b.dependency("miniaudio", .{});
    const mod = b.addModule("miniaudio", .{
        .optimize = config.optimize,
        .target = config.target,
        .link_libc = true,
    });

    const include = upstream.path(".");
    mod.addIncludePath(include);
    mod.addCSourceFile(.{ .file = upstream.path("miniaudio.c") });

    const lib = b.addLibrary(.{
        .name = "miniaudio",
        .root_module = mod,
    });
    lib.installHeadersDirectory(include, "", .{});
    return .{ .upstream = upstream, .artifact = lib };
}
