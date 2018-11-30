package(default_visibility = ["//visibility:public"])

cc_library(
    name = "rocket",
    srcs=[
        "rocket/base.h",
        "rocket/device.c",
        "rocket/device.h",
        "rocket/track.c",
        "ws2_32.lib",
        "rocket/track.h", ],
    hdrs=["rocket/sync.h"],
    strip_include_prefix = "rocket",
    #deps=[':socketlib'],
    )
