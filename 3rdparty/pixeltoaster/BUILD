load("//:copts.bzl", "RSR_DEFAULT_COPTS")

package(default_visibility = ["//visibility:public"])

cc_library(
    name = "pixeltoaster",
    srcs = [
        "PixelToaster.cpp",
        "PixelToasterCommon.h",
        "PixelToasterConversion.h",
        "PixelToasterWindows.h",
        ],
    hdrs = ["PixelToaster.h"],
    deps=[
        "//3rdparty/directx:sdk",
        ],
    copts = RSR_DEFAULT_COPTS,
    )

cc_binary(
    name = "example_floating_point",
    srcs = ["ExampleFloatingPoint.cpp"],
    deps = [
        ":pixeltoaster",
        ],
    copts = RSR_DEFAULT_COPTS,
    )
