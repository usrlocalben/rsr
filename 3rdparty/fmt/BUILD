# fmt-7.1.2

load("//:copts.bzl", "RSR_DEFAULT_COPTS")

package(default_visibility = ["//visibility:public"])

cc_library(
    name='fmt',
    hdrs=glob(["include/fmt/*.h"]),
    includes = ["include"],
    srcs=["src/format.cc", "src/os.cc",],
    copts = RSR_DEFAULT_COPTS,)
