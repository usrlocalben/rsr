load("//:copts.bzl", "RSR_DEFAULT_COPTS")

package(default_visibility = ["//visibility:public"])

cc_library(
    name = "irect",
    srcs = ["rmlg_irect.cxx", "rmlg_irect.hxx"],
    hdrs = ["rmlg_irect.hxx"],
    deps = [
        "//src/rml/rmlv:vec",
        ],
    copts = RSR_DEFAULT_COPTS,
    )

cc_library(
    name = "triangle",
    srcs = ["rmlg_triangle.cxx", "rmlg_triangle.hxx"],
    hdrs = ["rmlg_triangle.hxx"],
    deps = [
        "//src/rml/rmlv:vec",
        ],
    copts = RSR_DEFAULT_COPTS,
    )

cc_library(
    name = "pow2",
    srcs = ["rmlg_pow2.cxx", "rmlg_pow2.hxx"],
    hdrs = ["rmlg_pow2.hxx"],
    copts = RSR_DEFAULT_COPTS,
    )

cc_library(
    name = "noise",
    srcs = ["rmlg_noise.cxx"],
    hdrs = ["rmlg_noise.hxx"],
    copts = RSR_DEFAULT_COPTS,
    )
