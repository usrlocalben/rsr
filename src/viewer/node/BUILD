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
    name = "computed",
    srcs = ["computed.cxx"],
    deps = [
        "//3rdparty/exprtk",
        "//src/rcl/rclmt:jobsys",
        "//src/rcl/rclx:gason_util",
        "//src/rml/rmlv:vec",
        "//src/viewer:compile",
        ":i_value", ],
    copts = RSR_DEFAULT_COPTS,)

cc_library(
    name = "controller",
    srcs = ["controller.cxx"],
    deps = [
        "//3rdparty/lua",
        "//src/rcl/rclt:util",
        "//src/rcl/rclx:gason_util",
        "//src/rml/rmlv:vec",
        "//src/viewer:compile",
        ":i_controller",
        ":i_value", ],
    copts = RSR_DEFAULT_COPTS,)

cc_library(
    name = "auraforlaura",
    srcs = ["auraforlaura.cxx"],
    deps = [
        "//3rdparty/pixeltoaster",
        "//src/rcl/rclx:gason_util",
        "//src/rgl/rglv:gl",
        "//src/rgl/rglv:icosphere",
        "//src/rml/rmlm:mat4",
        "//src/viewer:compile",
        "//src/viewer:shaders",
        ":base",
        ":i_gl",
        ":i_material",
        ":i_value",],
    copts = RSR_DEFAULT_COPTS,)

cc_library(
    name = "mesh",
    srcs = ["mesh.cxx"],
    deps = [
        "//3rdparty/pixeltoaster",
        "//src/rcl/rcls:aligned_containers",
        "//src/rcl/rclx:gason_util",
        "//src/rgl/rglv:mesh",
        "//src/rgl/rglv:mesh_util",
        "//src/rml/rmlm:mat4",
        "//src/viewer:compile",
        "//src/viewer:shaders",
        ":base",
        ":i_gl",
        ":i_material", ],
    copts = RSR_DEFAULT_COPTS,)

cc_library(
    name = "writer",
    srcs = ["writer.cxx"],
    deps = [
        "//3rdparty/pixeltoaster",
        "//src/rcl/rcls:aligned_containers",
        "//src/rcl/rclx:gason_util",
        "//src/rgl/rglv:mesh",
        "//src/rgl/rglv:mesh_util",
        "//src/rml/rmlm:mat4",
        "//src/viewer:compile",
        "//src/viewer:fontloader",
        ":base",
        ":i_gl",
        ":i_material",
        ":i_value", ],
    copts = RSR_DEFAULT_COPTS,)

cc_library(
    name = "mc",
    srcs = ["mc.cxx"],
    deps = [
        "//src/rcl/rclmt:jobsys",
        "//src/rcl/rclx:gason_util",
        "//src/rgl/rglv:gl",
        "//src/rgl/rglv:marching_cubes",
        "//src/rgl/rglv:vao",
        "//src/rml/rmlm:mat4",
        "//src/rml/rmlv:mvec4",
        "//src/rml/rmlv:soa",
        "//src/rml/rmlv:vec",
        "//src/viewer:compile",
        "//src/viewer:shaders_envmap",
        ":base",
        ":i_gl",
        ":i_material",
        ":i_value", ],
    copts = RSR_DEFAULT_COPTS,)

cc_library(
    name = "many",
    srcs = ["many.cxx"],
    deps = [
        "//src/rcl/rclmt:jobsys",
        "//src/rcl/rclx:gason_util",
        "//src/rgl/rglv:gl",
        "//src/rgl/rglv:vao",
        "//src/rgl/rglv:mesh",
        "//src/rgl/rglv:mesh_util",
        "//src/rml/rmlm:mat4",
        "//src/rml/rmlv:mvec4",
        "//src/rml/rmlv:soa",
        "//src/rml/rmlv:vec",
        "//src/viewer:compile",
        "//src/viewer:shaders",
        ":base",
        ":i_gl",
        ":i_material",
        ":i_value", ],
    copts = RSR_DEFAULT_COPTS,)

cc_library(
    name = "plane",
    srcs = ["plane.cxx"],
    deps = [
        "//src/rcl/rclmt:jobsys",
        "//src/rcl/rcls:aligned_containers",
        "//src/rgl/rglv:math",
        "//src/rgl/rglv:mesh",
        "//src/rgl/rglv:vao",
        "//src/rml/rmlm:mat4",
        "//src/rml/rmlv:vec",
        "//src/viewer:compile",
        "//src/viewer:shaders",
        ":base",
        ":i_gl",
        ":i_material",
        ":i_value", ],
    copts = RSR_DEFAULT_COPTS,)

cc_library(
    name = "gllayer",
    srcs = ["gllayer.cxx"],
    deps = [
        "//src/rcl/rclma:framepool",
        "//src/rcl/rclmt:jobsys",
        "//src/rcl/rclx:gason_util",
        "//src/rgl/rglr:fragmentcursor",
        "//src/rgl/rglv:gl",
        "//src/rgl/rglv:gpu",
        "//src/rgl/rglv:gpu_impl",
        "//src/rml/rmlm:mat4",
        "//src/viewer:compile",
        ":base",
        ":i_camera",
        ":i_gl",
        ":i_layer",
        ":i_value",],
    copts = RSR_DEFAULT_COPTS,)

cc_library(
    name = "gpu",
    srcs = ["gpu.cxx"],
    deps = [
        "//src/rcl/rclmt:jobsys",
        "//src/rcl/rclx:gason_util",
        "//src/rgl/rglv:gpu",
        "//src/rgl/rglv:math",
        "//src/rml/rmlv:vec",
        "//src/viewer:compile",
        "//src/viewer:shaders",
        ":base",
        ":i_gpu",
        ":i_layer",
        ":i_value", ],
    copts = RSR_DEFAULT_COPTS,)

cc_library(
    name = "group",
    srcs = ["group.cxx"],
    deps = [
        "//src/rcl/rclx:gason_util",
        "//src/rcl/rclmt:jobsys",
        "//src/rgl/rglv:gl",
        "//src/rml/rmlm:mat4",
        "//src/viewer:compile",
        ":base",
        ":i_gl"],
    copts = RSR_DEFAULT_COPTS,)

cc_library(
    name = "i_camera",
    srcs = ["i_camera.cxx"],
    hdrs = ["i_camera.hxx"],
    deps = [
        "//src/rml/rmlm:mat4",
        ":base",],
    copts = RSR_DEFAULT_COPTS,)

cc_library(
    name = "i_gl",
    srcs = ["i_gl.cxx"],
    hdrs = ["i_gl.hxx"],
    deps = [
        "//src/rgl/rglv:gl",
        "//src/rml/rmlm:mat4",
        ":base",],
    copts = RSR_DEFAULT_COPTS,)

cc_library(
    name = "i_gpu",
    srcs = ["i_gpu.cxx"],
    hdrs = ["i_gpu.hxx"],
    deps = [
        "//src/rcl/rclmt:jobsys",
        "//src/rgl/rglr:canvas",
        "//src/rgl/rglv:gl",
        "//src/rml/rmlv:vec",
        ":base",],
    copts = RSR_DEFAULT_COPTS,)

cc_library(
    name = "i_layer",
    srcs = ["i_layer.cxx"],
    hdrs = ["i_layer.hxx"],
    deps = [
        "//src/rcl/rclmt:jobsys",
        "//src/rgl/rglv:gl",
        ":base",],
    copts = RSR_DEFAULT_COPTS,)

cc_library(
    name = "i_material",
    srcs = ["i_material.cxx"],
    hdrs = ["i_material.hxx"],
    deps = [
        "//src/rgl/rglv:gl",
        ":base",],
    copts = RSR_DEFAULT_COPTS,)

cc_library(
    name = "i_output",
    srcs = ["i_output.cxx"],
    hdrs = ["i_output.hxx"],
    deps = [
        "//src/rcl/rclmt:jobsys",
        "//src/rgl/rglr:canvas",
        "//src/rml/rmlv:vec",
        ":base",],
    copts = RSR_DEFAULT_COPTS,)

cc_library(
    name = "i_texture",
    srcs = ["i_texture.cxx"],
    hdrs = ["i_texture.hxx"],
    deps = [
        "//src/rgl/rglr:texture",
        ":base",],
    copts = RSR_DEFAULT_COPTS,)

cc_library(
    name = "i_canvas",
    srcs = ["i_canvas.cxx"],
    hdrs = ["i_canvas.hxx"],
    deps = [
        "//src/rgl/rglr:canvas",
        ":base",],
    copts = RSR_DEFAULT_COPTS,)

cc_library(
    name = "i_value",
    srcs = ["i_value.cxx"],
    hdrs = ["i_value.hxx"],
    deps = [
        "//src/rml/rmlv:vec",
        ":base",],
    copts = RSR_DEFAULT_COPTS,)

cc_library(
    name = "i_controller",
    srcs = ["i_controller.cxx"],
    hdrs = ["i_controller.hxx"],
    deps = [
        "//src/rml/rmlv:vec",],
    copts = RSR_DEFAULT_COPTS,)

cc_library(
    name = "image",
    srcs = ["image.cxx"],
    deps = [
        "//src/rcl/rclx:gason_util",
        "//src/rgl/rglr:texture",
        "//src/rgl/rglr:texture_load",
        "//src/viewer:compile",
        ":base",
        ":i_texture",],
    copts = RSR_DEFAULT_COPTS,)

cc_library(
    name = "layer_select",
    srcs = ["layer_select.cxx"],
    deps = [
        "//src/rcl/rclmt:jobsys",
        "//src/rcl/rclx:gason_util",
        "//src/rgl/rglv:gl",
        "//src/rml/rmlv:vec",
        "//src/viewer:compile",
        ":base",
        ":i_layer",
        ":i_value",],
    copts = RSR_DEFAULT_COPTS,)

cc_library(
    name = "material",
    srcs = ["material.cxx"],
    deps = [
        "//src/rcl/rclmt:jobsys",
        "//src/rcl/rclx:gason_util",
        "//src/rgl/rglr:texture",
        "//src/rgl/rglv:gl",
        "//src/viewer:compile",
        "//src/viewer:shaders",
        ":base",
        ":i_material",
        ":i_texture",
        ":i_value",],
    copts = RSR_DEFAULT_COPTS,)

cc_library(
    name = "multivalue",
    srcs = ["multivalue.cxx"],
    hdrs = ["multivalue.hxx"],
    deps = [
        "//src/rcl/rclmt:jobsys",
        ":base",
        ":i_value",],
    copts = RSR_DEFAULT_COPTS,)

cc_library(
    name = "noise",
    srcs = ["noise.cxx"],
    deps = [
        "//src/rcl/rclx:gason_util",
        "//src/rml/rmlg:noise",
        "//src/viewer:compile",
        ":i_value", ],
    copts = RSR_DEFAULT_COPTS,)

cc_library(
    name = "orthographic",
    srcs = ["orthographic.cxx"],
    deps = [
        "//src/rgl/rglv:math",
        "//src/rml/rmlm:mat4",
        "//src/viewer:compile",
        ":i_camera", ],
    copts = RSR_DEFAULT_COPTS,)

cc_library(
    name = "perspective",
    srcs = ["perspective.cxx"],
    deps = [
        "//src/rcl/rclx:gason_util",
        "//src/rgl/rglv:math",
        "//src/rml/rmlm:mat4",
        "//src/rml/rmlv:vec",
        "//src/viewer:compile",
        ":base",
        ":i_camera",
        ":i_value",],
    copts = RSR_DEFAULT_COPTS,)

cc_library(
    name = "truecolor",
    srcs = ["truecolor.cxx"],
    deps = [
        "//src/rcl/rclmt:jobsys",
        "//src/rcl/rclx:gason_util",
        "//src/rgl/rglr:canvas",
        "//src/rml/rmlv:vec",
        "//src/viewer:compile",
        "//src/viewer:shaders",
        ":base",
        ":i_gpu",
        ":i_output",
        ":i_value", ],
    copts = RSR_DEFAULT_COPTS,)

cc_library(
    name = "buffers",
    srcs = ["buffers.cxx"],
    deps = [
        "//src/rcl/rclmt:jobsys",
        "//src/rcl/rclx:gason_util",
        "//src/rgl/rglr:canvas",
        "//src/rml/rmlv:vec",
        "//src/viewer:compile",
        ":base",
        ":i_canvas",
        ":i_gpu" ],
    copts = RSR_DEFAULT_COPTS,)

cc_library(
    name = "rendertotexture",
    srcs = ["rendertotexture.cxx"],
    deps = [
        "//src/rcl/rclmt:jobsys",
        "//src/rcl/rclx:gason_util",
        "//src/rgl/rglr:canvas",
        "//src/rgl/rglr:texture",
        "//src/rml/rmlv:vec",
        "//src/viewer:compile",
        ":base",
        ":i_gpu",
        ":i_texture", ],
    copts = RSR_DEFAULT_COPTS,)

cc_library(
    name = "modify",
    srcs = ["modify.cxx"],
    deps = [
        "//src/rcl/rclma:framepool",
        "//src/rcl/rclmt:jobsys",
        "//src/rcl/rclx:gason_util",
        "//src/rgl/rglv:gl",
        "//src/rml/rmlm:mat4",
        "//src/rml/rmlv:vec",
        "//src/viewer:compile",
        ":base",
        ":i_gl",
        ":i_value", ],
    copts = RSR_DEFAULT_COPTS,)

cc_library(
    name = "uicamera",
    srcs = ["uicamera.cxx"],
    hdrs = ["uicamera.hxx"],
    deps = [
        "//src/rgl/rglv:camera",
        "//src/rml/rmlm:mat4",
        "//src/rgl/rglv:math",
        ":i_camera", ],
    copts = RSR_DEFAULT_COPTS,)

cc_library(
    name = "value",
    srcs = ["value.cxx"],
    deps = [
        "//src/rcl/rclx:gason_util",
        "//src/rml/rmlv:vec",
        "//src/viewer:compile",
        ":base",
        ":i_value",],
    copts = RSR_DEFAULT_COPTS,)

cc_library(
    name = "kawase",
    srcs = ["kawase.cxx"],
    deps = [
        "//src/rcl/rclmt:jobsys",
        "//src/rcl/rclx:gason_util",
        "//src/rgl/rglr:canvas",
        "//src/rgl/rglr:kawase",
        "//src/rml/rmlv:vec",
        "//src/viewer:compile",
        ":base",
        ":i_canvas", ],
    copts = RSR_DEFAULT_COPTS,)

cc_library(
    name = "glow",
    srcs = ["glow.cxx"],
    deps = [
        "//src/rcl/rclmt:jobsys",
        "//src/rcl/rclx:gason_util",
        "//src/rgl/rglr:algorithm",
        "//src/rgl/rglr:canvas",
        "//src/rgl/rglr:canvas_util",
        "//src/rml/rmlv:soa",
        "//src/rml/rmlv:vec",
        "//src/viewer:compile",
        ":base",
        ":i_canvas",
        ":i_output", ],
    copts = RSR_DEFAULT_COPTS,)
