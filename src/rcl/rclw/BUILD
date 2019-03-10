load("//:copts.bzl", "RSR_DEFAULT_COPTS")

package(default_visibility = ["//visibility:public"])

cc_library(
    name = "guid",
    srcs = ["rclw_guid.cxx"],
    hdrs = ["rclw_guid.hxx"],
    copts = RSR_DEFAULT_COPTS,
    linkopts = ['-DEFAULTLIB:ole32.lib'],
    )

cc_library(
    name = "smarti",
    srcs = ["rclw_smarti.cxx"],
    hdrs = ["rclw_smarti.hxx"],
    copts = RSR_DEFAULT_COPTS,
    )

cc_library(
    name = "winfile",
    srcs = ["rclw_winfile.cxx"],
    hdrs = ["rclw_winfile.hxx"],
    copts = RSR_DEFAULT_COPTS,
    )
