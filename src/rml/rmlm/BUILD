load("//:copts.bzl", "RSR_DEFAULT_COPTS")

package(default_visibility = ["//visibility:public"])

cc_library(
    name = "mat4",
    srcs = ["rmlm_mat4.cxx"],
    hdrs = ["rmlm_mat4.hxx"],
    deps = [
        "//3rdparty/fmt",
        "//src/rml/rmlv:vec",
        "//src/rml/rmlv:soa", ],
    copts = RSR_DEFAULT_COPTS,)

cc_test(
    name = "mat4_unittest",
    srcs = ["rmlm_mat4.t.cxx"],
    deps = [
        "//3rdparty/googletest:gtest_main",
        "//src/rml/rmlv:vec",
        ":mat4", ],
    copts = RSR_DEFAULT_COPTS,)

cc_library(
    name = "soa",
    srcs = ["rmlm_soa.cxx"],
    hdrs = ["rmlm_soa.hxx"],
    deps = [
        "//src/rml/rmlv:mvec4",
        "//src/rml/rmlv:soa",
        ":mat4", ],
    copts = RSR_DEFAULT_COPTS,)

cc_library(
    name = "matrixstack",
    srcs = ["rmlm_matrixstack.cxx"],
    hdrs = ["rmlm_matrixstack.hxx"],
    deps = [],
    copts = RSR_DEFAULT_COPTS,)
