load("//:copts.bzl", "RSR_DEFAULT_COPTS")

package(default_visibility = ["//visibility:public"])

cc_library(
    name = "rocket_editor",
    srcs = [
        "rocket/base.h",
        "rocket/device.c",
        "rocket/device.h",
        "rocket/sync.h",
        "rocket/track.c",
        "rocket/track.h", ],
    hdrs = ["rocket/sync.h"],
    strip_include_prefix = "rocket",
    defines = ["WIN32",],
    copts = ["/wd4244"] + RSR_DEFAULT_COPTS,
    linkopts = ['-DEFAULTLIB:ws2_32.lib'],)


cc_library(
    name = "rocket_player",
    srcs = [
        "rocket/base.h",
        "rocket/device.c",
        "rocket/device.h",
        "rocket/sync.h",
        "rocket/track.c",
        "rocket/track.h", ],
    hdrs = ["rocket/sync.h"],
    strip_include_prefix = "rocket",
    defines = ["WIN32", "SYNC_PLAYER"],
    copts = ["/wd4244",] + RSR_DEFAULT_COPTS)

# linkopts = ['-DEFAULTLIB:ws2_32.lib'],)
