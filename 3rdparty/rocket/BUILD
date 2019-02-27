load("//:copts.bzl", "RSR_DEFAULT_COPTS")

package(default_visibility = ["//visibility:public"])

cc_library(
    name = "rocket",
    srcs = [
        "rocket/base.h",
        "rocket/device.c",
        "rocket/device.h",
        "rocket/sync.h",
        "rocket/track.c",
        "rocket/track.h",
        ],
    hdrs = ["rocket/sync.h"],
    strip_include_prefix = "rocket",
    copts = ["/DWIN32",] + RSR_DEFAULT_COPTS,
    linkopts = ['-DEFAULTLIB:ws2_32.lib'],
    )
