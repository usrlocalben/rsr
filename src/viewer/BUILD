load("//:copts.bzl", "RSR_DEFAULT_COPTS")

package(default_visibility = ["//visibility:public"])

cc_binary(
    name = "viewer",
    srcs = [
        "rqv.m.cxx",
        ":resource_data",
        ],
    deps = [
        "//3rdparty/fmt",
        "//3rdparty/gason",
        "//src/rcl/rclma:framepool",
        "//src/rcl/rclmt:jobsys",
        "//src/rcl/rclx:gason_util",
        "//src/rcl/rclx:jsonfile",
        ":app",
        ":resource_decl",
        ],
    copts = RSR_DEFAULT_COPTS,
    linkopts = ["-SUBSYSTEM:CONSOLE",],
    )

cc_binary(
    name = "demo",
    srcs = [
        "rqv.m.cxx",
        ":resource_data",
        ],
    deps = [
        "//3rdparty/fmt",
        "//3rdparty/gason",
        "//src/rcl/rclma:framepool",
        "//src/rcl/rclmt:jobsys",
        "//src/rcl/rclx:gason_util",
        "//src/rcl/rclx:jsonfile",
        ":app",
        ":resource_decl",
        ],
    copts = RSR_DEFAULT_COPTS,
    linkopts = ["-SUBSYSTEM:WINDOWS",],
    )

cc_library(
    name = "app",
    srcs = ["rqv_app.cxx"],
    hdrs = ["rqv_app.hxx"],
    deps = [
        "//3rdparty/fmt",
        "//3rdparty/gason",
        "//3rdparty/pixeltoaster",
        "//src/ral/ralio:audio_controller",
        "//src/ral/rals:sync_controller",
        "//src/rcl/rclma:framepool",
        "//src/rcl/rclmt:barrier",
        "//src/rcl/rclmt:jobsys",
        "//src/rcl/rclr:algorithm",
        "//src/rcl/rclx:gason_util",
        "//src/rcl/rclx:jsonfile",
        "//src/rgl/rglr:display_mode",
        "//src/rgl/rglr:pixeltoaster_util",
        "//src/rgl/rglr:profont",
        "//src/rgl/rglr:texture_store",
        "//src/rgl/rglv:camera",
        "//src/rgl/rglv:material",
        "//src/rgl/rglv:mesh_store",
        "//src/rml/rmls:bench",
        "//src/rml/rmlv:vec",
        ":compile",
        ":jobsys_vis",
        ":node_camera",
        ":node_output",
        ":node_value",
        ],
    copts = RSR_DEFAULT_COPTS,
    )

cc_library(
    name = "compile",
    srcs = ["rqv_compile.cxx"],
    hdrs = ["rqv_compile.hxx"],
    deps = [
        "//3rdparty/gason",
        "//3rdparty/pixeltoaster",
        "//src/rcl/rclt:util",
        "//src/rcl/rclx:gason_util",
        "//src/rgl/rglr:texture",
        "//src/rgl/rglr:texture_load",
        "//src/rgl/rglv:mesh_store",
        ":node_camera",
        ":node_computed",
        ":node_fx_auraforlaura",
        ":node_fx_carpet",
        ":node_fx_foo",
        ":node_fx_mc",
        ":node_fx_xyquad",
        ":node_gl",
        ":node_gpu",
        ":node_material",
        ":node_render",
        ":node_translate",
        ":node_value",
        ],
    copts = RSR_DEFAULT_COPTS,
    )


cc_library(
    name = "cpu_colors",
    srcs = ["rqv_cpu_colors.cxx"],
    hdrs = ["rqv_cpu_colors.hxx"],
    deps = [
        "//3rdparty/pixeltoaster",
        ],
    copts = RSR_DEFAULT_COPTS,
    )


cc_library(
    name = "jobsys_vis",
    srcs = ["rqv_jobsys_vis.cxx"],
    hdrs = ["rqv_jobsys_vis.hxx"],
    deps = [
        "//3rdparty/pixeltoaster",
        "//src/rgl/rglr:canvas",
        "//src/rcl/rclmt:jobsys",
        ],
    copts = RSR_DEFAULT_COPTS,
    )


cc_library(
    name = "node_base",
    srcs = ["rqv_node_base.cxx"],
    hdrs = ["rqv_node_base.hxx"],
    deps = [
        "//src/rcl/rclmt:jobsys",
        ],
    copts = RSR_DEFAULT_COPTS,
    )


cc_library(
    name = "node_camera",
    srcs = ["rqv_node_camera.cxx"],
    hdrs = ["rqv_node_camera.hxx"],
    deps = [
        "//src/rml/rmlm:mat4",
        "//src/rml/rmlv:vec",
        "//src/rgl/rglv:camera",
        "//src/rgl/rglv:math",
        ":node_base",
        ":node_value",
        ],
    copts = RSR_DEFAULT_COPTS,
    )

cc_library(
    name = "node_computed",
    srcs = ["rqv_node_computed.cxx"],
    hdrs = ["rqv_node_computed.hxx"],
    deps = [
        "//3rdparty/exprtk",
        "//src/rcl/rclmt:jobsys",
        "//src/rml/rmlv:vec",
        ":node_value",
        ],
    copts = RSR_DEFAULT_COPTS,
    )


cc_library(
    name = "node_fx_foo",
    srcs = ["rqv_node_fx_foo.cxx"],
    hdrs = ["rqv_node_fx_foo.hxx"],
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
        ":node_base",
        ":node_gl",
        ":node_material",
        ":shaders",
        ],
    copts = RSR_DEFAULT_COPTS,
    )

cc_library(
    name = "node_fx_mc",
    srcs = ["rqv_node_fx_mc.cxx"],
    hdrs = ["rqv_node_fx_mc.hxx"],
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
        ":node_base",
        ":node_gl",
        ":node_material",
        ":node_value",
        ":shaders",
        ],
    copts = RSR_DEFAULT_COPTS,
    )

cc_library(
    name = "node_fx_xyquad",
    srcs = ["rqv_node_fx_xyquad.cxx"],
    hdrs = ["rqv_node_fx_xyquad.hxx"],
    deps = [
        "//src/rcl/rclmt:jobsys",
        "//src/rcl/rcls:aligned_containers",
        "//src/rgl/rglr:texture",
        "//src/rgl/rglv:gl",
        "//src/rgl/rglv:mesh",
        "//src/rgl/rglv:vao",
        "//src/rml/rmlm:mat4",
        "//src/rml/rmlv:vec",
        ":node_base",
        ":node_gl",
        ":node_material",
        ":node_value",
        ":shaders",
        ],
    copts = RSR_DEFAULT_COPTS,
    )

cc_library(
    name = "node_gl",
    srcs = ["rqv_node_gl.cxx"],
    hdrs = ["rqv_node_gl.hxx"],
    deps = [
        "//src/rcl/rclma:framepool",
        "//src/rcl/rclmt:jobsys",
        "//src/rgl/rglv:camera",
        "//src/rgl/rglv:gl",
        "//src/rgl/rglv:math",
        "//src/rml/rmlm:mat4",
        ":node_base",
        ":node_camera",
        ":node_value",
        ],
    copts = RSR_DEFAULT_COPTS,
    )

cc_library(
    name = "node_output",
    srcs = ["rqv_node_output.cxx"],
    hdrs = ["rqv_node_output.hxx"],
    deps = [
        "//src/rcl/rclmt:jobsys",
        "//src/rgl/rglr:canvas",
        "//src/rml/rmlv:vec",
        ":node_base",
        ],
    copts = RSR_DEFAULT_COPTS,
    )

cc_library(
    name = "node_gpu",
    srcs = ["rqv_node_gpu.cxx"],
    hdrs = ["rqv_node_gpu.hxx"],
    deps = [
        "//3rdparty/ryg-srgb",
        "//src/rcl/rclma:framepool",
        "//src/rcl/rclmt:jobsys",
        "//src/rcl/rclr:algorithm",
        "//src/rgl/rglv:gpu",
        "//src/rgl/rglv:math",
        "//src/rml/rmlm:mat4",
        "//src/rml/rmlv:vec",
        ":node_base",
        ":node_camera",
        ":node_gl",
        ":node_value",
        ":shaders",
        ],
    copts = RSR_DEFAULT_COPTS,
    )

cc_library(
    name = "node_render",
    srcs = ["rqv_node_render.cxx"],
    hdrs = ["rqv_node_render.hxx"],
    deps = [
        "//src/rcl/rclmt:jobsys",
        "//src/rgl/rglr:canvas",
        "//src/rgl/rglr:texture",
        "//src/rgl/rglr:texture_store",
        "//src/rgl/rglv:mesh",
        "//src/rgl/rglv:mesh_store",
        "//src/rml/rmlv:vec",
        ":node_base",
        ":node_gpu",
        ":node_material",
        ":node_output",
        ":shaders",
        ],
    copts = RSR_DEFAULT_COPTS,
    )

cc_library(
    name = "node_translate",
    srcs = ["rqv_node_translate.cxx"],
    hdrs = ["rqv_node_translate.hxx"],
    deps = [
        "//src/rcl/rclma:framepool",
        "//src/rcl/rclmt:jobsys",
        "//src/rgl/rglv:gl",
        "//src/rml/rmlm:mat4",
        "//src/rml/rmlv:vec",
        ":node_base",
        ":node_gl",
        ":node_value",
        ],
    copts = RSR_DEFAULT_COPTS,
    )

cc_library(
    name = "node_value",
    srcs = ["rqv_node_value.cxx"],
    hdrs = ["rqv_node_value.hxx"],
    deps = [
        "//src/rcl/rclmt:jobsys",
        "//src/rml/rmlv:vec",
        ":node_base",
        ],
    copts = RSR_DEFAULT_COPTS,
    )

cc_library(
    name = "shaders",
    srcs = ["rqv_shaders.cxx"],
    hdrs = ["rqv_shaders.hxx"],
    deps = [
        "//src/rgl/rglv:fragment",
        "//src/rgl/rglv:gpu",
        "//src/rgl/rglv:math",
        "//src/rgl/rglv:triangle",
        "//src/rml/rmlm:mat4",
        "//src/rml/rmlv:soa",
        "//src/rml/rmlv:vec",
        ],
    copts = RSR_DEFAULT_COPTS,
    )

cc_library(
    name = "resource_decl",
    srcs = ["resource.cxx"],
    hdrs = ["resource.h"],
    copts = RSR_DEFAULT_COPTS,
    )

genrule(
    name = "resource_data",
    srcs = ["resources.rc", "resource.h"],
    outs = ["resources.o"],
    cmd = " ".join([
        "$(location compile-resources.sh)",
        "$(location resources.rc)",
        "$(location resource.h)",
        "$@",
        "$(location gen_vsenv_stub.sh)",
        ]),
    output_to_bindir = 1,
    tools = [
        "compile-resources.sh",
        "gen_vsenv_stub.sh",
        ],
    )

cc_library(
    name = "node_material",
    srcs = ["rqv_node_material.cxx"],
    hdrs = ["rqv_node_material.hxx"],
    deps = [
        "//src/rcl/rclmt:jobsys",
        "//src/rgl/rglr:texture",
        "//src/rgl/rglv:gl",
        "//src/rml/rmlm:mat4",
        ":node_base",
        ":node_gl",
        ":shaders",
        ],
    copts = RSR_DEFAULT_COPTS,
    )

cc_library(
    name = "node_fx_auraforlaura",
    srcs = ["rqv_node_fx_auraforlaura.cxx"],
    hdrs = ["rqv_node_fx_auraforlaura.hxx"],
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
        ":node_base",
        ":node_gl",
        ":node_material",
        ":shaders",
        ],
    copts = RSR_DEFAULT_COPTS,
    )

cc_library(
    name = "node_fx_carpet",
    srcs = ["rqv_node_fx_carpet.cxx"],
    hdrs = ["rqv_node_fx_carpet.hxx"],
    deps = [
        "//src/rcl/rclmt:jobsys",
        "//src/rcl/rcls:aligned_containers",
        "//src/rcl/rcls:timer",
        "//src/rgl/rglr:texture",
        "//src/rgl/rglv:gl",
        "//src/rgl/rglv:mesh",
        "//src/rgl/rglv:vao",
        "//src/rml/rmlm:mat4",
        ":node_base",
        ":node_gl",
        ":node_material",
        ":shaders",
        ],
    copts = RSR_DEFAULT_COPTS,
    )
