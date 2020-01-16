# github.com/foonathan/type_safe @ 1271d5f

load("//:copts.bzl", "RSR_DEFAULT_COPTS")

package(default_visibility = ["//visibility:public"])

cc_library(
    name='type_safe',
    hdrs=glob(["include/type_safe/*.hpp",
               "include/type_safe/detail/*.hpp"]),
    includes = ["include"],
    deps=["//3rdparty/debug_assert"],
    copts = RSR_DEFAULT_COPTS,)
