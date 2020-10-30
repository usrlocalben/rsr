load("//:copts.bzl", "RSR_DEFAULT_COPTS")

package(default_visibility = ["//visibility:public"])

cc_binary(
    name = "lua",
    srcs = ["lua.c"],
    deps = [":lib"],)

cc_binary(
    name = "luac",
    srcs = ["luac.c"],
    deps = [":lib"],)

cc_library(
    name = "lib",
    hdrs = [
        "lauxlib.h",
        "lua.h",
        "lua.hpp",
        "luaconf.h",
        "lualib.h",
        ],
    srcs = [
        "lauxlib.c", "lauxlib.h",
        "lapi.c", "lapi.h",
        "lcode.c", "lcode.h",
        "lctype.c", "lctype.h",
        "ldebug.c", "ldebug.h",
        "ldo.c", "ldo.h",
        "ldump.c",
        "lfunc.c", "lfunc.h",
        "lgc.c", "lgc.h",
        "linit.c",
        "ljumptab.h",
        "llex.c", "llex.h",
        "llimits.h",
        "lmem.c", "lmem.h",
        "lobject.c", "lobject.h",
        "lopcodes.c", "lopcodes.h",
        "lopnames.h",
        "lparser.c", "lparser.h",
        "lprefix.h",
        "lstate.c", "lstate.h",
        "lstring.c", "lstring.h",
        "ltable.c", "ltable.h",
        "ltm.c", "ltm.h",
        "lua.c", "lua.h",
        "lundump.c", "lundump.h",
        "lvm.c", "lvm.h",
        "lzio.c", "lzio.h",

        # lua stdlib
        "lbaselib.c",
        "lcorolib.c",
        "ldblib.c",
        "liolib.c",
        "lmathlib.c",
        "loadlib.c",
        "loslib.c",
        "lstrlib.c",
        "ltablib.c",
        "lutf8lib.c",],
    copts = RSR_DEFAULT_COPTS + ["/wd4244","/wd4310","/wd4702","/wd4701"])
