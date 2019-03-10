load("//:copts.bzl", "RSR_DEFAULT_COPTS")

package(default_visibility = ["//visibility:public"])

cc_library(
    name = "base",
    srcs = ["base.cxx"],
    hdrs = ["base.hxx"],
    deps = [
        "//src/rcl/rclmt:jobsys", ],
    copts = RSR_DEFAULT_COPTS,)


cc_library(
    name = "camera",
    srcs = ["camera.cxx"],
    hdrs = ["camera.hxx"],
    deps = [
        "//src/rml/rmlm:mat4",
        "//src/rml/rmlv:vec",
        "//src/rgl/rglv:camera",
        "//src/rgl/rglv:math",
        ":base",
        ":value", ],
    copts = RSR_DEFAULT_COPTS,)


cc_library(
    name = "computed",
    srcs = ["computed.cxx"],
    hdrs = ["computed.hxx"],
    deps = [
        "//3rdparty/exprtk",
        "//src/rcl/rclmt:jobsys",
        "//src/rml/rmlv:vec",
        ":value", ],
    copts = RSR_DEFAULT_COPTS,)


cc_library(
    name = "fx_foo",
    srcs = ["fx_foo.cxx"],
    hdrs = ["fx_foo.hxx"],
    deps = [
        "//3rdparty/pixeltoaster",
        "//src/rcl/rclmt:jobsys",
        "//src/rcl/rcls:aligned_containers",
        "//src/rcl/rcls:timer",
        "//src/rgl/rglr:texture",
        "//src/rgl/rglv:gl",
        "//src/rgl/rglv:mesh",
        "//src/rgl/rglv:vao",
        "//src/rml/rmlm:mat4",
        "//src/viewer:shaders",
        ":base",
        ":gl",
        ":material", ],
    copts = RSR_DEFAULT_COPTS,)


cc_library(
    name = "fx_mc",
    srcs = ["fx_mc.cxx"],
    hdrs = ["fx_mc.hxx"],
    deps = [
        "//src/rcl/rclmt:jobsys",
        "//src/rcl/rcls:aligned_containers",
        "//src/rcl/rcls:timer",
        "//src/rgl/rglr:texture",
        "//src/rgl/rglv:gl",
        "//src/rgl/rglv:marching_cubes",
        "//src/rml/rmlm:mat4",
        "//src/rml/rmlv:vec",
        "//src/rml/rmlv:soa",
        "//src/rml/rmlv:mvec4",
        "//src/viewer:shaders",
        ":base",
        ":gl",
        ":material",
        ":value", ],
    copts = RSR_DEFAULT_COPTS,)


cc_library(
    name = "fx_xyquad",
    srcs = ["fx_xyquad.cxx"],
    hdrs = ["fx_xyquad.hxx"],
    deps = [
        "//src/rcl/rclmt:jobsys",
        "//src/rcl/rcls:aligned_containers",
        "//src/rgl/rglr:texture",
        "//src/rgl/rglv:gl",
        "//src/rgl/rglv:mesh",
        "//src/rgl/rglv:vao",
        "//src/rml/rmlm:mat4",
        "//src/rml/rmlv:vec",
        "//src/viewer:shaders",
        ":base",
        ":gl",
        ":material",
        ":value", ],
    copts = RSR_DEFAULT_COPTS,)


cc_library(
    name = "gl",
    srcs = ["gl.cxx"],
    hdrs = ["gl.hxx"],
    deps = [
        "//src/rcl/rclma:framepool",
        "//src/rcl/rclmt:jobsys",
        "//src/rgl/rglv:camera",
        "//src/rgl/rglv:gl",
        "//src/rgl/rglv:math",
        "//src/rml/rmlm:mat4",
        ":base",
        ":camera",
        ":value", ],
    copts = RSR_DEFAULT_COPTS,)


cc_library(
    name = "output",
    srcs = ["output.cxx"],
    hdrs = ["output.hxx"],
    deps = [
        "//src/rcl/rclmt:jobsys",
        "//src/rgl/rglr:canvas",
        "//src/rml/rmlv:vec",
        ":base", ],
    copts = RSR_DEFAULT_COPTS,)


cc_library(
    name = "gpu",
    srcs = ["gpu.cxx"],
    hdrs = ["gpu.hxx"],
    deps = [
        "//3rdparty/ryg-srgb",
        "//src/rcl/rclma:framepool",
        "//src/rcl/rclmt:jobsys",
        "//src/rcl/rclr:algorithm",
        "//src/rgl/rglv:gpu",
        "//src/rgl/rglv:math",
        "//src/rml/rmlm:mat4",
        "//src/rml/rmlv:vec",
        "//src/viewer:shaders",
        ":base",
        ":camera",
        ":gl",
        ":value", ],
    copts = RSR_DEFAULT_COPTS,)


cc_library(
    name = "render",
    srcs = ["render.cxx"],
    hdrs = ["render.hxx"],
    deps = [
        "//src/rcl/rclmt:jobsys",
        "//src/rgl/rglr:canvas",
        "//src/rgl/rglr:texture",
        "//src/rgl/rglr:texture_store",
        "//src/rgl/rglv:mesh",
        "//src/rgl/rglv:mesh_store",
        "//src/rml/rmlv:vec",
        "//src/viewer:shaders",
        ":base",
        ":gpu",
        ":material",
        ":output", ],
    copts = RSR_DEFAULT_COPTS,)


cc_library(
    name = "translate",
    srcs = ["translate.cxx"],
    hdrs = ["translate.hxx"],
    deps = [
        "//src/rcl/rclma:framepool",
        "//src/rcl/rclmt:jobsys",
        "//src/rgl/rglv:gl",
        "//src/rml/rmlm:mat4",
        "//src/rml/rmlv:vec",
        ":base",
        ":gl",
        ":value", ],
    copts = RSR_DEFAULT_COPTS,)


cc_library(
    name = "value",
    srcs = ["value.cxx"],
    hdrs = ["value.hxx"],
    deps = [
        "//src/rcl/rclmt:jobsys",
        "//src/rml/rmlv:vec",
        ":base", ],
    copts = RSR_DEFAULT_COPTS,)


cc_library(
    name = "material",
    srcs = ["material.cxx"],
    hdrs = ["material.hxx"],
    deps = [
        "//src/rcl/rclmt:jobsys",
        "//src/rgl/rglr:texture",
        "//src/rgl/rglv:gl",
        "//src/rml/rmlm:mat4",
        "//src/viewer:shaders",
        ":base",
        ":gl", ],
    copts = RSR_DEFAULT_COPTS,)


cc_library(
    name = "fx_auraforlaura",
    srcs = ["fx_auraforlaura.cxx"],
    hdrs = ["fx_auraforlaura.hxx"],
    deps = [
        "//3rdparty/pixeltoaster",
        "//src/rcl/rclmt:jobsys",
        "//src/rcl/rcls:aligned_containers",
        "//src/rcl/rcls:timer",
        "//src/rgl/rglr:texture",
        "//src/rgl/rglv:gl",
        "//src/rgl/rglv:mesh",
        "//src/rgl/rglv:vao",
        "//src/rml/rmlm:mat4",
        "//src/viewer:shaders",
        ":base",
        ":gl",
        ":material", ],
    copts = RSR_DEFAULT_COPTS,)


cc_library(
    name = "fx_carpet",
    srcs = ["fx_carpet.cxx"],
    hdrs = ["fx_carpet.hxx"],
    deps = [
        "//src/rcl/rclmt:jobsys",
        "//src/rcl/rcls:aligned_containers",
        "//src/rcl/rcls:timer",
        "//src/rgl/rglr:texture",
        "//src/rgl/rglv:gl",
        "//src/rgl/rglv:mesh",
        "//src/rgl/rglv:vao",
        "//src/rml/rmlm:mat4",
        "//src/viewer:shaders",
        ":base",
        ":gl",
        ":material", ],
    copts = RSR_DEFAULT_COPTS,)
