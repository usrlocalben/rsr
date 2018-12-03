load("//:copts.bzl", "RSR_DEFAULT_COPTS")

package(default_visibility = ["//visibility:public"])

cc_library(
    name = "util",
    srcs = ["rclt_util.cxx"],
    hdrs = ["rclt_util.hxx"],
    copts = RSR_DEFAULT_COPTS,
    )

cc_test(
    name = "util_unittest",
    srcs = ["rclt_util.t.cxx"],
    deps = [
        "//3rdparty/googletest:gtest_main",
        ":util",
        ],
    copts = RSR_DEFAULT_COPTS,
    )
