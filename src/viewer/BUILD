load("//:copts.bzl", "RSR_DEFAULT_COPTS")

package(default_visibility = ["//visibility:public"])


main_srcs = [
    "main.cxx",
    ":resource_data",]
main_deps_minus_app = [
        "//3rdparty/fmt",
        "//3rdparty/gason",
        "//src/rcl/rclma:framepool",
        "//src/rcl/rclmt:jobsys",
        "//src/rcl/rclx:gason_util",
        "//src/rcl/rclx:jsonfile",
        ":resource_decl",]
cc_binary(
    name = "viewer",
    srcs = main_srcs,
    deps = main_deps_minus_app + [":app_silent",],
    copts = RSR_DEFAULT_COPTS,
    linkopts = ["-SUBSYSTEM:CONSOLE",],)
cc_binary(
    name = "editor",
    srcs = main_srcs,
    deps = main_deps_minus_app + ["app_music_editor",],
    copts = RSR_DEFAULT_COPTS,
    linkopts = ["-SUBSYSTEM:CONSOLE",],)
cc_binary(
    name = "demo",
    srcs = main_srcs,
    deps = main_deps_minus_app + ["app_music_player",],
    copts = RSR_DEFAULT_COPTS,
    linkopts = ["-SUBSYSTEM:WINDOWS",],)


app_srcs = ["app.cxx"]
app_hdrs = ["app.hxx"]
app_deps = [
    "//3rdparty/fmt",
    "//3rdparty/gason",
    "//3rdparty/pixeltoaster",
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
    "//src/viewer/node:i_output",
    "//src/viewer/node:uicamera",
    "//src/viewer/node:multivalue",
    ":compile",
    ":jobsys_vis",
    ":node_library", ]
cc_library(
    name = "app_silent", srcs = app_srcs, hdrs = app_hdrs,
    deps = app_deps,
    copts = RSR_DEFAULT_COPTS,)
cc_library(
    name = "app_music_editor", srcs = app_srcs, hdrs = app_hdrs,
    deps = app_deps + [
        "//src/ral/ralio:audio_controller",
        "//src/ral/rals:sync_controller_editor",],
    copts = ["/DENABLE_MUSIC",] + RSR_DEFAULT_COPTS)
cc_library(
    name = "app_music_player", srcs = app_srcs, hdrs = app_hdrs,
    deps = app_deps + [
        "//src/ral/ralio:audio_controller",
        "//src/ral/rals:sync_controller_player",],
    copts = ["/DENABLE_MUSIC", "/DSYNC_PLAYER",] + RSR_DEFAULT_COPTS)


cc_library(
    name = "compile",
    srcs = ["compile.cxx"],
    hdrs = ["compile.hxx"],
    deps = [
        "//3rdparty/gason",
        "//src/rcl/rclt:util",
        "//src/rcl/rclx:gason_util",
        "//src/rgl/rglv:mesh_store",
        "//src/viewer/node:base",],
    copts = RSR_DEFAULT_COPTS,)


cc_library(
    name = "node_library",
    deps = [
        "//src/viewer/node:computed",
        "//src/viewer/node:fx_auraforlaura",
        "//src/viewer/node:fx_carpet",
        "//src/viewer/node:fx_foo",
        "//src/viewer/node:fx_mc",
        "//src/viewer/node:fx_xyquad",
        "//src/viewer/node:gllayer",
        "//src/viewer/node:gpu",
        "//src/viewer/node:group",
        "//src/viewer/node:image",
        "//src/viewer/node:layer_select",
        "//src/viewer/node:material",
        "//src/viewer/node:noise",
        "//src/viewer/node:orthographic",
        "//src/viewer/node:perspective",
        "//src/viewer/node:render",
        "//src/viewer/node:rendertotexture",
        "//src/viewer/node:translate",
        "//src/viewer/node:value", ],
    linkopts = [
        "-WHOLEARCHIVE:$(execpath //src/viewer/node:computed)",
        "-WHOLEARCHIVE:$(execpath //src/viewer/node:fx_auraforlaura)",
        "-WHOLEARCHIVE:$(execpath //src/viewer/node:fx_carpet)",
        "-WHOLEARCHIVE:$(execpath //src/viewer/node:fx_foo)",
        "-WHOLEARCHIVE:$(execpath //src/viewer/node:fx_mc)",
        "-WHOLEARCHIVE:$(execpath //src/viewer/node:fx_xyquad)",
        "-WHOLEARCHIVE:$(execpath //src/viewer/node:gllayer)",
        "-WHOLEARCHIVE:$(execpath //src/viewer/node:gpu)",
        "-WHOLEARCHIVE:$(execpath //src/viewer/node:group)",
        "-WHOLEARCHIVE:$(execpath //src/viewer/node:image)",
        "-WHOLEARCHIVE:$(execpath //src/viewer/node:layer_select)",
        "-WHOLEARCHIVE:$(execpath //src/viewer/node:material)",
        "-WHOLEARCHIVE:$(execpath //src/viewer/node:noise)",
        "-WHOLEARCHIVE:$(execpath //src/viewer/node:orthographic)",
        "-WHOLEARCHIVE:$(execpath //src/viewer/node:perspective)",
        "-WHOLEARCHIVE:$(execpath //src/viewer/node:render)",
        "-WHOLEARCHIVE:$(execpath //src/viewer/node:rendertotexture)",
        "-WHOLEARCHIVE:$(execpath //src/viewer/node:translate)",
        "-WHOLEARCHIVE:$(execpath //src/viewer/node:value)",],
    copts = RSR_DEFAULT_COPTS,)


cc_library(
    name = "cpu_colors",
    srcs = ["cpu_colors.cxx"],
    hdrs = ["cpu_colors.hxx"],
    deps = [
        "//3rdparty/pixeltoaster", ],
    copts = RSR_DEFAULT_COPTS,)


cc_library(
    name = "jobsys_vis",
    srcs = ["jobsys_vis.cxx"],
    hdrs = ["jobsys_vis.hxx"],
    deps = [
        "//3rdparty/pixeltoaster",
        "//src/rgl/rglr:canvas",
        "//src/rcl/rclmt:jobsys", ],
    copts = RSR_DEFAULT_COPTS,)


cc_library(
    name = "shaders",
    srcs = ["shaders.cxx"],
    hdrs = ["shaders.hxx"],
    deps = [
        "//src/rgl/rglv:fragment",
        "//src/rgl/rglv:gpu_shaders",
        "//src/rgl/rglv:interpolate",
        "//src/rgl/rglv:math",
        "//src/rgl/rglv:triangle",
        "//src/rml/rmlm:mat4",
        "//src/rml/rmlv:soa",
        "//src/rml/rmlv:vec", ],
    copts = RSR_DEFAULT_COPTS,)


cc_library(
    name = "resource_decl",
    srcs = ["resource.cxx"],
    hdrs = ["resource.h"],
    copts = RSR_DEFAULT_COPTS,)


genrule(
    name = "resource_data",
    srcs = ["resources.rc", "resource.h"],
    outs = ["resources.o"],
    cmd = " ".join([
        "$(location compile-resources.sh)",
        "$(location resources.rc)",
        "$(location resource.h)",
        "$@",
        "$(location gen_vsenv_stub.sh)", ]),
    output_to_bindir = 1,
    tools = [
        "compile-resources.sh",
        "gen_vsenv_stub.sh", ],)
