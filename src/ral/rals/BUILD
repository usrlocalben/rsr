load("//:copts.bzl", "RSR_DEFAULT_COPTS")

package(default_visibility = ["//visibility:public"])

cc_library(
    name = "sync_controller_player",
    srcs = ["rals_sync_controller.cxx"],
    hdrs = ["rals_sync_controller.hxx"],
    deps = [
        "//3rdparty/rocket:rocket_player",
        "//src/ral/ralio:audio_controller", ],
    copts = ["/DSYNC_PLAYER",] + RSR_DEFAULT_COPTS,)


cc_library(
    name = "sync_controller_editor",
    srcs = ["rals_sync_controller.cxx"],
    hdrs = ["rals_sync_controller.hxx"],
    deps = [
        "//3rdparty/rocket:rocket_editor",
        "//src/ral/ralio:audio_controller", ],
    copts = RSR_DEFAULT_COPTS,)
