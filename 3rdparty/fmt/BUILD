package(default_visibility = ["//visibility:public"])

cc_library(
    name='fmt',
    hdrs=glob(["include/fmt/*.h"]),
    srcs=[
        "src/format.cc",
        "src/posix.cc",],
    strip_include_prefix = "include",)
