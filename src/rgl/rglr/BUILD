load("//:copts.bzl", "RSR_DEFAULT_COPTS")

package(default_visibility = ["//visibility:public"])

cc_library(
    name = "blend",
    srcs = ["rglr_blend.cxx"],
    hdrs = ["rglr_blend.hxx"],
    deps = [
        "//src/rml/rmlv:soa",
        "//src/rml/rmlv:mvec4",
        ],
    copts = RSR_DEFAULT_COPTS,
    )

cc_library(
    name = "canvas",
    srcs = ["rglr_canvas.cxx"],
    hdrs = ["rglr_canvas.hxx"],
    deps = [
        "//3rdparty/pixeltoaster",
        "//src/rcl/rcls:aligned_containers",
        "//src/rml/rmlg:irect",
        "//src/rml/rmlv:soa",
        "//src/rml/rmlv:vec",
        ],
    copts = RSR_DEFAULT_COPTS,
    )

cc_library(
    name = "canvas_util",
    srcs = ["rglr_canvas_util.cxx"],
    hdrs = ["rglr_canvas_util.hxx"],
    deps = [
        "//3rdparty/ryg-srgb",
        "//src/rml/rmlg:irect",
        "//src/rml/rmlv:mvec4",
        "//src/rml/rmlv:soa",
        ":canvas",
        ],
    copts = RSR_DEFAULT_COPTS,
    )

cc_library(
    name = "algorithm",
    srcs = ["rglr_algorithm.cxx"],
    hdrs = ["rglr_algorithm.hxx"],
    deps = [
        "//3rdparty/pixeltoaster",
        "//src/rml/rmlg:irect",
        "//src/rml/rmlv:mvec4",
        "//src/rml/rmlv:soa",
        ":canvas",
        ],
    copts = RSR_DEFAULT_COPTS,
    )

cc_library(
    name = "display_mode",
    srcs = ["rglr_display_mode.cxx"],
    hdrs = ["rglr_display_mode.hxx"],
    copts = RSR_DEFAULT_COPTS,
    )

cc_library(
    name = "kawase",
    srcs = ["rglr_kawase.cxx"],
    hdrs = ["rglr_kawase.hxx"],
    deps = [
        "//src/rml/rmlv:mvec4",
        ":canvas",
        ],
    copts = RSR_DEFAULT_COPTS,
    )

cc_library(
    name = "pixeltoaster_util",
    srcs = ["rglr_pixeltoaster_util.cxx"],
    hdrs = ["rglr_pixeltoaster_util.hxx"],
    deps = [
        "//3rdparty/pixeltoaster",
        ":canvas",
        ],
    copts = RSR_DEFAULT_COPTS,
    )

cc_library(
    name = "profont",
    srcs = ["rglr_profont.cxx"],
    hdrs = ["rglr_profont.hxx"],
    deps = [
        "//3rdparty/fmt",
        "//3rdparty/picopng",
        "//3rdparty/pixeltoaster",
        "//src/rcl/rclr:algorithm",
        "//src/rml/rmlv:vec",
        ":canvas",
        ],
    copts = RSR_DEFAULT_COPTS,
    )

cc_library(
    name = "texture",
    srcs = ["rglr_texture.cxx"],
    hdrs = ["rglr_texture.hxx"],
    deps = [
        "//3rdparty/pixeltoaster",
        "//src/rcl/rcls:aligned_containers",
        "//src/rml/rmlg:pow2",
        "//src/rml/rmlv:mvec4",
        "//src/rml/rmlv:vec",
        ],
    copts = RSR_DEFAULT_COPTS,
    )

cc_library(
    name = "texture_store",
    srcs = ["rglr_texture_store.cxx"],
    hdrs = ["rglr_texture_store.hxx"],
    deps = [
        "//3rdparty/fmt",
        "//src/rcl/rcls:file",
        ":texture",
        ":texture_load",
        ],
    copts = RSR_DEFAULT_COPTS,
    )

cc_library(
    name = "texture_sampler",
    srcs = ["rglr_texture_sampler.cxx"],
    hdrs = ["rglr_texture_sampler.hxx"],
    deps = [
        "//3rdparty/pixeltoaster",
        "//src/rml/rmlg:pow2",
        "//src/rml/rmlv:mvec4",
        "//src/rml/rmlv:soa",
        "//src/rml/rmlv:vec",
        ],
    copts = RSR_DEFAULT_COPTS,
    )

cc_library(
    name = "texture_load",
    srcs = ["rglr_texture_load.cxx"],
    hdrs = ["rglr_texture_load.hxx"],
    deps = [
        "//3rdparty/fmt",
        "//3rdparty/picopng",
        "//3rdparty/pixeltoaster",
        "//3rdparty/ryg-srgb",
        "//src/rcl/rcls:aligned_containers",
        "//src/rcl/rcls:file",
        "//src/rml/rmlg:pow2",
        "//src/rml/rmlv:vec",
        ":texture",
        ],
    copts = RSR_DEFAULT_COPTS,
    )

cc_library(
    name = "fragmentcursor",
    srcs = ["rglr_fragmentcursor.cxx"],
    hdrs = ["rglr_fragmentcursor.hxx"],
    deps = [
        "//src/rgl/rglr:canvas",
        "//src/rml/rmlv:soa",
        "//src/rml/rmlv:vec",
        ],
    copts = RSR_DEFAULT_COPTS,
    )
