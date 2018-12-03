load("//:copts.bzl", "RSR_DEFAULT_COPTS")

package(default_visibility = ["//visibility:public"])

cc_library(
    name = "framepool",
    srcs = ["rclma_framepool.cxx"],
    hdrs = ["rclma_framepool.hxx"],
    deps = [
        "//src/rcl/rclmt:jobsys",
        ],
    copts = RSR_DEFAULT_COPTS,
    )
