load("//:copts.bzl", "RSR_DEFAULT_COPTS")

package(default_visibility = ["//visibility:public"])

cc_library(
    name = "gason_util",
    srcs = ["rclx_gason_util.cxx"],
    hdrs = ["rclx_gason_util.hxx"],
    deps = [
        "//3rdparty/gason",
        "//src/rml/rmlv:vec",
        ],
    copts = RSR_DEFAULT_COPTS,
    )

cc_library(
    name = "jsonfile",
    srcs = ["rclx_jsonfile.cxx"],
    hdrs = ["rclx_jsonfile.hxx"],
    deps = [
        "//3rdparty/gason",
        "//src/rcl/rcls:file",
        ],
    copts = RSR_DEFAULT_COPTS,
    )
