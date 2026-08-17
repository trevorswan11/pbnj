pub const base_sources = [_][]const u8{
    "context.c",
    "egl_context.c",
    "init.c",
    "input.c",
    "monitor.c",
    "null_init.c",
    "null_joystick.c",
    "null_monitor.c",
    "null_window.c",
    "osmesa_context.c",
    "platform.c",
    "vulkan.c",
    "window.c",
};

pub const linux_sources = [_][]const u8{
    "linux_joystick.c",
    "posix_module.c",
    "posix_poll.c",
    "posix_thread.c",
    "posix_time.c",
    "xkb_unicode.c",
};

pub const linux_wl_sources = [_][]const u8{
    "wl_init.c",
    "wl_monitor.c",
    "wl_window.c",
};

pub const linux_x11_sources = [_][]const u8{
    "glx_context.c",
    "x11_init.c",
    "x11_monitor.c",
    "x11_window.c",
};

pub const windows_sources = [_][]const u8{
    "wgl_context.c",
    "win32_init.c",
    "win32_joystick.c",
    "win32_module.c",
    "win32_monitor.c",
    "win32_thread.c",
    "win32_time.c",
    "win32_window.c",
};

pub const macos_sources = [_][]const u8{
    // C sources
    "macos_time.c",
    "posix_module.c",
    "posix_thread.c",

    // ObjC sources
    "cocoa_init.m",
    "cocoa_joystick.m",
    "cocoa_monitor.m",
    "cocoa_window.m",
    "nsgl_context.m",
};
