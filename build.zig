const std = @import("std");
const zon = @import("build.zig.zon");
pub const stdx = @import("stdx");

const stb = @import("third-party/stb.zig");
const sokol = @import("third-party/sokol.zig");
const miniaudio = @import("third-party/miniaudio.zig");

pub fn build(b: *std.Build) !void {
    const optimize = b.standardOptimizeOption(.{
        .preferred_optimize_mode = .ReleaseFast,
    });

    const profile = b.option(bool, "profile", "Enable chromium tracing") orelse false;
    const stdx_dep = b.dependency("stdx", .{
        .target = b.graph.host,
        .optimize = optimize,
        .profile = profile,
        .building_for_dep = true,
        .run_cdb_gen = false,
    });

    var compiler_flags: stdx.ArrayList([]const u8) = .fromSlice(b, &stdx.utils.base_cxx_flags);
    compiler_flags.appendSlice(&.{ "-DMAGIC_ENUM_RANGE_MAX=255", "-DSPDLOG_COMPILED_LIB" });
    const dist_flags: []const []const u8 = &.{ "-DNDEBUG", "-DPBNJ_DIST" };

    var package_flags = compiler_flags.clone();
    package_flags.appendSlice(dist_flags);
    stdx.CDBGenerator.addCdbFlags(b, &compiler_flags.wrapped);

    switch (optimize) {
        .Debug => compiler_flags.appendSlice(&.{ "-g", "-DPBNJ_DEBUG" }),
        .ReleaseSafe => compiler_flags.appendSlice(&.{"-DPBNJ_RELEASE"}),
        .ReleaseFast, .ReleaseSmall => compiler_flags.appendSlice(dist_flags),
    }

    const install_tests_only = b.option(
        bool,
        "install-tests-only",
        "Install tests without running them (default: false)",
    ) orelse false;

    const cdb_gen: *stdx.CDBGenerator = .init(b);
    var cdb_steps: stdx.ArrayList(*std.Build.Step) = .init(b);
    const artifacts = try addArtifacts(b, .{
        .optimize = optimize,
        .cxx_flags = compiler_flags.wrapped.items,
        .cdb_steps = &cdb_steps,
        .install_tests_only = install_tests_only,
        .stdx_dep = stdx_dep,
        .profile = profile,
    });
    for (cdb_steps.wrapped.items) |cdb_step| cdb_gen.step.dependOn(cdb_step);

    try addToolingSteps(b, .{
        .cdb_gen = cdb_gen,
        .cppcheck = stdx_dep.artifact("cppcheck"),
    });

    if (stdx.KcovBuilder.allowedTarget(b.graph.host)) {
        if (artifacts.tests) |tests| {
            var include_patterns: stdx.ArrayList([]const u8) = .init(b);
            const libraries = [_][]const u8{
                "audio",    "network",
                "services", "ui",
            };
            for (libraries) |library| {
                include_patterns.append(b.fmt("lib/{s}/src", .{library}));
                include_patterns.append(b.fmt("lib/{s}/include", .{library}));
            }

            var configs: stdx.ArrayList(stdx.steps.RunKcovConfig) = .init(b);
            for (tests.unit_suites) |suite| {
                configs.append(.{
                    .artifact = suite.artifact,
                    .include_patterns = include_patterns.wrapped.items,
                });
            }

            try stdx.steps.addCoverage(b, .{
                .curl = stdx_dep.artifact("execurl"),
                .kcov = stdx_dep.artifact("kcov"),
                .run_configs = configs.wrapped.items,
            });
        }
    }
}

const ArtifactConfig = struct {
    /// Used for the directory lookup and artifact name
    name: []const u8,
    target: std.Build.ResolvedTarget,
    optimize: std.builtin.OptimizeMode,
    cxx_flags: []const []const u8,
    cdb_steps: ?*stdx.ArrayList(*std.Build.Step),
    config_h: *std.Build.Step.ConfigHeader,
    stdx_dep: *std.Build.Dependency,
    auto_install: bool,
    /// The library's include and source dirs are implicitly included
    include_paths: []const std.Build.LazyPath = &.{},
    system_include_paths: []const std.Build.LazyPath = &.{},
    /// libstdx & spdlog are implicitly linked
    link_libraries: []const *std.Build.Step.Compile = &.{},
    profile: bool = false,
    install_dir: ?[]const u8 = null,
    install_only: bool = false,
    libsupport: ?*std.Build.Step.Compile = null,
    libtesthelpers: ?*std.Build.Step.Compile = null,

    pub fn libraryInclude(b: *std.Build, name: []const u8) struct { []const u8, std.Build.LazyPath } {
        const path = b.pathJoin(&.{ Library.library_root, name, Library.include_root });
        return .{ path, b.path(path) };
    }

    fn tryAddStrongLib(
        compile: ?*std.Build.Step.Compile,
        root: []const u8,
        include_paths: *stdx.ArrayList(std.Build.LazyPath),
        link_libraries: *stdx.ArrayList(*std.Build.Step.Compile),
    ) void {
        if (compile) |artifact| {
            link_libraries.append(artifact);
            include_paths.append(libraryInclude(artifact.step.owner, root).@"1");
        }
    }

    pub fn tryAddSupport(
        self: *const ArtifactConfig,
        include_paths: *stdx.ArrayList(std.Build.LazyPath),
        link_libraries: *stdx.ArrayList(*std.Build.Step.Compile),
    ) void {
        tryAddStrongLib(self.libsupport, "support", include_paths, link_libraries);
    }

    pub fn tryAddTestHelpers(
        self: *const ArtifactConfig,
        include_paths: *stdx.ArrayList(std.Build.LazyPath),
        link_libraries: *stdx.ArrayList(*std.Build.Step.Compile),
    ) void {
        tryAddStrongLib(self.libtesthelpers, "testhelpers", include_paths, link_libraries);
    }

    /// Creates a new configuration with a new name, paths, and link libraries
    pub fn with(self: *const ArtifactConfig, name: []const u8, altered: struct {
        include_paths: []const std.Build.LazyPath = &.{},
        system_include_paths: []const std.Build.LazyPath = &.{},
        link_libraries: []const *std.Build.Step.Compile = &.{},
    }) ArtifactConfig {
        return .{
            .name = name,
            .target = self.target,
            .optimize = self.optimize,
            .cxx_flags = self.cxx_flags,
            .cdb_steps = self.cdb_steps,
            .config_h = self.config_h,
            .stdx_dep = self.stdx_dep,
            .auto_install = self.auto_install,
            .include_paths = altered.include_paths,
            .system_include_paths = altered.system_include_paths,
            .link_libraries = altered.link_libraries,
            .profile = self.profile,
            .install_dir = self.install_dir,
            .install_only = self.install_only,
            .libsupport = self.libsupport,
            .libtesthelpers = self.libtesthelpers,
        };
    }
};

const Library = struct {
    const library_root = "lib/";
    const include_root = "include/";
    const src_root = "src/";

    b: *std.Build,
    config_h: *std.Build.Step.ConfigHeader,

    include: std.Build.LazyPath,
    src: std.Build.LazyPath,
    files: []const []const u8,

    step: *std.Build.Step,
    artifact: *std.Build.Step.Compile,

    pub fn init(b: *std.Build, config: ArtifactConfig) Library {
        const include, const include_path = ArtifactConfig.libraryInclude(b, config.name);
        const src = b.pathJoin(&.{ library_root, config.name, src_root });
        const src_paths = stdx.utils.collectFiles(
            b,
            src,
            .{ .allowed_extensions = &.{".cc"} },
        ) catch @panic("OOM");

        var link_libraries: stdx.ArrayList(*std.Build.Step.Compile) = .fromSlice(b, config.link_libraries);
        link_libraries.append(config.stdx_dep.artifact("stdx"));

        var include_paths: stdx.ArrayList(std.Build.LazyPath) = .fromSlice(b, config.include_paths);
        include_paths.appendSlice(&.{ include_path, b.path(src) });
        config.tryAddSupport(&include_paths, &link_libraries);

        const lib = b.addLibrary(.{
            .name = config.name,
            .root_module = stdx.utils.createModule(b, .{
                .target = config.target,
                .optimize = config.optimize,
                .include_paths = include_paths.wrapped.items,
                .system_include_paths = config.system_include_paths,
                .cxx = .{
                    .files = src_paths,
                    .flags = config.cxx_flags,
                },
                .config_headers = &.{config.config_h},
                .link_libraries = link_libraries.wrapped.items,
            }),
        });
        stdx.Dependency.addFrameworkSearchPaths(lib.root_module, config.target);
        lib.installHeadersDirectory(include_path, "", .{ .include_extensions = &.{".hh"} });
        if (config.cdb_steps) |cdb_steps| cdb_steps.append(&lib.step);
        if (config.auto_install) b.installArtifact(lib);

        return .{
            .b = b,
            .config_h = config.config_h,
            .include = include_path,
            .src = b.path(src),
            .files = stdx.utils.collectFiles(
                b,
                include,
                .{ .allowed_extensions = &.{".hh"}, .extra_files = src_paths },
            ) catch @panic("OOM"),
            .step = &lib.step,
            .artifact = lib,
        };
    }
};

const Test = struct {
    const tests_root = "tests/";
    const fuzz_root = "fuzz/";

    step: *std.Build.Step,
    artifact: *std.Build.Step.Compile,

    pub fn init(b: *std.Build, config: ArtifactConfig) Test {
        _, const include_path = ArtifactConfig.libraryInclude(b, config.name);
        const tests_dir = b.pathJoin(&.{ tests_root, config.name });
        var include_paths: stdx.ArrayList(std.Build.LazyPath) = .fromSlice(b, config.include_paths);
        include_paths.appendSlice(&.{ b.path(tests_dir), include_path });

        var link_libraries: stdx.ArrayList(*std.Build.Step.Compile) = .fromSlice(b, config.link_libraries);
        config.tryAddSupport(&include_paths, &link_libraries);
        config.tryAddTestHelpers(&include_paths, &link_libraries);

        const step_name = b.fmt("test-{s}", .{config.name});
        const desc = b.fmt("Build/run {s} tests", .{config.name});

        const test_artifact = stdx.builders.strappedTest(b, .{
            .target = config.target,
            .optimize = config.optimize,
            .stdx = .{ .dep = config.stdx_dep },
            .cxx_files = stdx.utils.collectFiles(b, tests_dir, .{}) catch @panic("OOM"),
            .cxx_flags = config.cxx_flags,
            .profile = config.profile,
            .include_paths = include_paths.wrapped.items,
            .link_libraries = link_libraries.wrapped.items,
            .config_headers = &.{config.config_h},
            .executable_config = .{
                .name = config.name,
                .behavior = .{
                    .installable = .{
                        .cmd_name = step_name,
                        .cmd_desc = desc,
                        .install_dir = config.install_dir,
                        .install_only = config.install_only,
                    },
                },
            },
        });
        stdx.Dependency.addFrameworkSearchPaths(test_artifact.root_module, config.target);
        if (config.cdb_steps) |cdb| cdb.append(&test_artifact.step);

        return .{
            .step = &test_artifact.step,
            .artifact = test_artifact,
        };
    }
};

const Tests = struct {
    b: *std.Build,
    unit_suites: []const Test,

    pub fn configure(self: *const Tests, config: struct {
        test_install_dir: ?[]const u8,
        install_only: bool,
    }) !void {
        const b = self.b;
        const test_step = b.step("test", "Build/run all unit tests");
        for (self.unit_suites) |suite| {
            _ = stdx.utils.ExecutableBehavior.installArtifact(
                b,
                suite.artifact,
                test_step,
                config.test_install_dir,
                config.install_only,
            );
        }
    }
};

const version_str = zon.version;
const version = std.SemanticVersion.parse(version_str) catch @compileError("Malformed version");

fn addArtifacts(b: *std.Build, config: struct {
    target: ?std.Build.ResolvedTarget = null,
    optimize: std.builtin.OptimizeMode,
    cxx_flags: []const []const u8,
    cdb_steps: ?*stdx.ArrayList(*std.Build.Step),
    exe_override_behavior: ?stdx.utils.ExecutableBehavior = null,
    auto_install: bool = true,
    packaging: bool = false,
    install_tests_only: bool = false,
    stdx_dep: *std.Build.Dependency,
    profile: bool = false,
}) !struct {
    libsupport: Library,
    libaudio: Library,
    libnetwork: Library,
    libservices: Library,
    libui: Library,
    pbnj: *std.Build.Step.Compile,
    tests: ?Tests,
} {
    const target = config.target orelse b.graph.host;
    const config_h = b.addConfigHeader(.{ .include_path = "pbnj/config.h" }, .{
        .PBNJ_VERSION_STR = version_str,
        .PBNJ_VERSION_MAJOR = @as(i64, version.major),
        .PBNJ_VERSION_MINOR = @as(i64, version.minor),
        .PBNJ_VERSION_PATCH = @as(i64, version.patch),
        .PBNJ_VERSION_PRE = version.pre orelse "",
        .PBNJ_GIT_INFO = stdx.utils.getGitInfo(b),
        .PBNJ_WINDOWS = target.result.os.tag == .windows,
        .PBNJ_LINUX = target.result.os.tag == .linux,
        .PBNJ_APPLE = target.result.os.tag == .macos,
    });

    const dep_config: stdx.Dependency.Config = .{
        .optimize = config.optimize,
        .target = target,
    };
    const stb_dep = stb.build(b, dep_config);
    const sokol_dep = sokol.build(b, dep_config);
    const ma_dep = miniaudio.build(b, dep_config);

    var base_lib_config: ArtifactConfig = .{
        .name = undefined,
        .target = target,
        .optimize = config.optimize,
        .cxx_flags = config.cxx_flags,
        .cdb_steps = config.cdb_steps,
        .config_h = config_h,
        .stdx_dep = config.stdx_dep,
        .auto_install = config.auto_install,
        .profile = config.profile,
    };

    const libcurl = config.stdx_dep.artifact("curl");
    const libsupport: Library = .init(b, base_lib_config.with("support", .{
        .link_libraries = &.{ stb_dep.artifact, ma_dep.artifact },
    }));
    base_lib_config.libsupport = libsupport.artifact;

    const libaudio: Library = .init(b, base_lib_config.with("audio", .{}));
    const libnetwork: Library = .init(b, base_lib_config.with("network", .{}));
    const libservices: Library = .init(b, base_lib_config.with("services", .{
        .link_libraries = &.{libcurl},
    }));
    const libui: Library = .init(b, base_lib_config.with("ui", .{
        .link_libraries = &.{ sokol_dep.artifact, stb_dep.artifact },
    }));

    const all_pbnj_libraries = [_]*std.Build.Step.Compile{
        libsupport.artifact,  libaudio.artifact, libnetwork.artifact,
        libservices.artifact, libui.artifact,
    };

    var exe_link_libraries: stdx.ArrayList(*std.Build.Step.Compile) = .fromSlice(b, &all_pbnj_libraries);
    exe_link_libraries.append(config.stdx_dep.artifact("stdx"));

    const pbnj = stdx.utils.createExecutable(b, .{
        .target = target,
        .optimize = config.optimize,
        .cxx = .{
            .files = &.{"pbnj/main.cc"},
            .flags = config.cxx_flags,
        },
        .link_libraries = exe_link_libraries.wrapped.items,
    }, .{
        .name = "pbnj",
        .behavior = config.exe_override_behavior orelse .{
            .installable = .{
                .cmd_name = "run",
                .cmd_desc = "Run pbnj with provided command line arguments",
            },
        },
    });
    stdx.Dependency.addFrameworkSearchPaths(pbnj.root_module, target);
    if (config.auto_install) b.installArtifact(pbnj);
    if (config.cdb_steps) |cdb_steps| cdb_steps.append(&pbnj.step);

    const disable_console = config.optimize != .Debug;
    if (target.result.os.tag == .windows and disable_console) {
        pbnj.subsystem = .windows;
    }

    var tests: ?Tests = null;
    if (config.target == null) {
        const test_install_dir: ?[]const u8 = if (config.auto_install) "tests" else null;

        var testhelpers_config = base_lib_config.with("testhelpers", .{
            .link_libraries = &.{config.stdx_dep.artifact("catch2")},
        });
        testhelpers_config.auto_install = false;
        const libtesthelpers: Library = .init(b, testhelpers_config);

        const base_test_config: ArtifactConfig = .{
            .name = undefined,
            .target = target,
            .optimize = config.optimize,
            .cxx_flags = config.cxx_flags,
            .cdb_steps = config.cdb_steps,
            .config_h = config_h,
            .stdx_dep = config.stdx_dep,
            .auto_install = config.auto_install,
            .profile = config.profile,
            .install_dir = test_install_dir,
            .install_only = config.install_tests_only,
            .libsupport = libsupport.artifact,
            .libtesthelpers = libtesthelpers.artifact,
        };

        var unit_suites: stdx.ArrayList(Test) = .init(b);
        unit_suites.append(.init(b, base_test_config.with("support", .{})));
        unit_suites.append(.init(b, base_test_config.with("audio", .{
            .link_libraries = &.{libaudio.artifact},
        })));
        unit_suites.append(.init(b, base_test_config.with("network", .{
            .link_libraries = &.{libnetwork.artifact},
        })));
        unit_suites.append(.init(b, base_test_config.with("services", .{
            .link_libraries = &.{libservices.artifact},
        })));
        unit_suites.append(.init(b, base_test_config.with("ui", .{
            .link_libraries = &.{libui.artifact},
        })));

        tests = .{
            .b = b,
            .unit_suites = unit_suites.wrapped.items,
        };
        try tests.?.configure(.{
            .test_install_dir = test_install_dir,
            .install_only = config.install_tests_only,
        });
    }

    return .{
        .libsupport = libsupport,
        .libaudio = libaudio,
        .libnetwork = libnetwork,
        .libservices = libservices,
        .libui = libui,
        .pbnj = pbnj,
        .tests = tests,
    };
}

const counted_extensions = [_][]const u8{ ".cc", ".hh", ".zig" };

fn addToolingSteps(b: *std.Build, config: struct {
    cdb_gen: *stdx.CDBGenerator,
    cppcheck: *std.Build.Step.Compile,
}) !void {
    const tooling_paths: stdx.steps.FmtPaths = .{
        .cxx = blk: {
            var paths: stdx.ArrayList([]const u8) = .init(b);
            try stdx.utils.collectFilesInto(b, "lib", .{ .allowed_extensions = &.{ ".hh", ".cc" } }, &paths);
            try stdx.utils.collectFilesInto(b, "tests", .{ .allowed_extensions = &.{ ".hh", ".cc" } }, &paths);
            try stdx.utils.collectFilesInto(b, "pbnj", .{}, &paths);
            break :blk paths.wrapped.items;
        },
        .zig = &.{"build.zig"},
    };

    _ = stdx.steps.addFmt(b, .{
        .paths = tooling_paths,
        .formatter = .{ .version = "21.1.8" },
    }) catch {};

    _ = stdx.steps.addCppcheck(b, .{
        .cppcheck = config.cppcheck,
        .cdb_gen = config.cdb_gen,
    });

    var counted_files: stdx.ArrayList([]const u8) = .init(b);
    counted_files.appendSlice(tooling_paths.cxx);
    counted_files.appendSlice(tooling_paths.zig);
    _ = stdx.LOCCounter.init(b, counted_files.wrapped.items);
}
