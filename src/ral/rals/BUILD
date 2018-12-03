load("//:copts.bzl", "RSR_DEFAULT_COPTS")

package(default_visibility = ["//visibility:public"])

cc_library(
    name = "sync_controller",
    srcs = ["rals_sync_controller.cxx"],
    hdrs = ["rals_sync_controller.hxx"],
    deps = [
        "//3rdparty/rocket",
        "//src/ral/ralio:audio_controller",
        ],
    copts = RSR_DEFAULT_COPTS,
    )
