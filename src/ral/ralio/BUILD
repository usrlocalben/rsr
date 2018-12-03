load("//:copts.bzl", "RSR_DEFAULT_COPTS")

package(default_visibility = ["//visibility:public"])

cc_library(
    name="audio_controller",
    srcs=["ralio_audio_controller.cxx"],
    hdrs=["ralio_audio_controller.hxx"],
    deps=[
        "//3rdparty/bass",
        ],
    copts = RSR_DEFAULT_COPTS,
    )
