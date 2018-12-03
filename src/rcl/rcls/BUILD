load("//:copts.bzl", "RSR_DEFAULT_COPTS")

package(default_visibility = ["//visibility:public"])

cc_library(
    name = "timer",
    srcs = ["rcls_timer.cxx"],
    hdrs = ["rcls_timer.hxx"],
    copts = RSR_DEFAULT_COPTS,
    )

cc_library(
    name = "file",
    srcs = ["rcls_file.cxx"],
    hdrs = ["rcls_file.hxx"],
    deps = [
        "//src/rcl/rclt:util",
        ],
    copts = RSR_DEFAULT_COPTS,
    )

cc_library(
    name = "aligned_containers",
    srcs = ["rcls_aligned_containers.cxx"],
    hdrs = ["rcls_aligned_containers.hxx"],
    deps = [
        "//3rdparty/aligned_allocator",
        ],
    copts = RSR_DEFAULT_COPTS,
    )
