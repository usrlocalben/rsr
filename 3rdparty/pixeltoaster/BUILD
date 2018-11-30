package(default_visibility = ["//visibility:public"])

cc_library(
    name = "pixeltoaster",
    srcs = [
        "PixelToaster.cpp",
        "PixelToasterCommon.h",
        "PixelToasterConversion.h",
        "PixelToasterWindows.h", ],
    hdrs = ["PixelToaster.h"],
    deps=[
        "//3rdparty/directx:sdk", ],)

cc_binary(
    name='example_floating_point',
    srcs=['ExampleFloatingPoint.cpp'],
    deps=[':pixeltoaster'],)
