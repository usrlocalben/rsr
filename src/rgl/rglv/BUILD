load("//:copts.bzl", "RSR_DEFAULT_COPTS")

package(default_visibility = ["//visibility:public"])

cc_library(
    name = "packed_stream",
    srcs = ["rglv_packed_stream.cxx"],
    hdrs = ["rglv_packed_stream.hxx"],
    deps = [
        "//src/rml/rmlv:vec",
        ],
    copts = RSR_DEFAULT_COPTS,
    )

cc_test(
    name = "packed_stream_unittest",
    srcs = ["rglv_packed_stream.t.cxx"],
    deps = [
        "//3rdparty/fmt",
        "//3rdparty/googletest:gtest_main",
        "//src/rml/rmlv:vec",
        ":packed_stream",
        ],
    copts = RSR_DEFAULT_COPTS,
    )

cc_library(
    name = "gpu",
    srcs = ["rglv_gpu.cxx"],
    hdrs = ["rglv_gpu.hxx"],
    deps = [
        "//3rdparty/fmt",
        "//src/rcl/rclmt:jobsys",
        "//src/rcl/rclr:algorithm",
        "//src/rcl/rcls:aligned_containers",
        "//src/rgl/rglr:blend",
        "//src/rgl/rglr:canvas",
        "//src/rgl/rglr:canvas_util",
        "//src/rgl/rglr:texture_sampler",
        "//src/rml/rmlg:irect",
        "//src/rml/rmlg:triangle",
        "//src/rml/rmlm:mat4",
        "//src/rml/rmlv:soa",
        "//src/rml/rmlv:vec",
        ":clipper",
        ":fragment",
        ":gl",
        ":gpu_protocol",
        ":gpu_shaders",
        ":interpolate",
        ":math",
        ":packed_stream",
        ":triangle",
        ":vao",
        ],
    copts = RSR_DEFAULT_COPTS,
    )

cc_library(
    name = "gpu_protocol",
    srcs = ["rglv_gpu_protocol.cxx"],
    hdrs = ["rglv_gpu_protocol.hxx"],
    copts = RSR_DEFAULT_COPTS,
    )

cc_library(
    name = "gl",
    srcs = ["rglv_gl.cxx"],
    hdrs = ["rglv_gl.hxx"],
    deps = [
        "//3rdparty/pixeltoaster",
        "//src/rgl/rglr:canvas",
        "//src/rml/rmlm:mat4",
        "//src/rml/rmlv:vec",
        ":gpu_protocol",
        ":packed_stream",
        ":vao",
        ],
    copts = RSR_DEFAULT_COPTS,
    )

cc_library(
    name = "camera",
    srcs = ["rglv_camera.cxx"],
    hdrs = ["rglv_camera.hxx"],
    deps = [
        "//src/rml/rmlm:mat4",
        "//src/rml/rmlv:vec",
        ":math",
        ],
    copts = RSR_DEFAULT_COPTS,
    )

cc_library(
    name = "material",
    srcs = ["rglv_material.cxx"],
    hdrs = ["rglv_material.hxx"],
    deps = [
        "//src/rcl/rcls:aligned_containers",
        "//src/rml/rmlv:vec",
        ],
    copts = RSR_DEFAULT_COPTS,
    )

cc_library(
    name = "mesh",
    srcs = ["rglv_mesh.cxx"],
    hdrs = ["rglv_mesh.hxx"],
    deps = [
        "//3rdparty/tuple_hash",
        "//src/rcl/rclr:algorithm",
        "//src/rcl/rcls:aligned_containers",
        "//src/rml/rmlv:vec",
        ":vao",
        ],
    copts = RSR_DEFAULT_COPTS,
    )

cc_library(
    name = "math",
    srcs = ["rglv_math.cxx"],
    hdrs = ["rglv_math.hxx"],
    deps = [
        "//src/rml/rmlm:mat4",
        "//src/rml/rmlv:soa",
        "//src/rml/rmlv:vec",
        ],
    copts = RSR_DEFAULT_COPTS,
    )

cc_library(
    name = "vao",
    srcs = ["rglv_vao.cxx"],
    hdrs = ["rglv_vao.hxx"],
    deps = [
        "//src/rcl/rcls:aligned_containers",
        "//src/rml/rmlv:soa",
        "//src/rml/rmlv:vec",
        ],
    copts = RSR_DEFAULT_COPTS,
    )

cc_test(
    name = "math_unittest",
    srcs = ["rglv_math.t.cxx"],
    deps = [
        "//3rdparty/googletest:gtest_main",
        "//src/rml/rmlm:mat4",
        "//src/rml/rmlv:vec",
        ":math",
        ],
    copts = RSR_DEFAULT_COPTS,
    )

cc_library(
    name = "clipper",
    srcs = ["rglv_clipper.cxx"],
    hdrs = ["rglv_clipper.hxx"],
    deps = [
        "//src/rcl/rcls:aligned_containers",
        "//src/rml/rmlv:soa",
        "//src/rml/rmlv:vec",
        "//src/rml/rmlv:mvec4",
        ],
    copts = RSR_DEFAULT_COPTS,
    )

cc_library(
    name = "obj",
    srcs = ["rglv_obj.cxx"],
    hdrs = ["rglv_obj.hxx"],
    deps = [
        "//src/rcl/rcls:file",
        "//src/rcl/rclt:util",
        "//src/rml/rmlv:vec",
        ":material",
        ":mesh",
        ],
    copts = RSR_DEFAULT_COPTS,
    )

cc_library(
    name = "mesh_store",
    srcs = ["rglv_mesh_store.cxx"],
    hdrs = ["rglv_mesh_store.hxx"],
    deps = [
        "//src/rcl/rcls:aligned_containers",
        "//src/rcl/rcls:file",
        "//src/rgl/rglr:texture_store",
        ":material",
        ":mesh",
        ":obj",
        ],
    copts = RSR_DEFAULT_COPTS,
    )

cc_library(
    name = "marching_cubes",
    srcs = ["rglv_marching_cubes.cxx"],
    hdrs = ["rglv_marching_cubes.hxx"],
    deps = [
        "//src/rml/rmlv:vec",
        ":vao",
        ],
    copts = RSR_DEFAULT_COPTS,
    )

cc_library(
    name = "fragment",
    srcs = ["rglv_fragment.cxx"],
    hdrs = ["rglv_fragment.hxx"],
    deps = [
        "//src/rml/rmlv:mvec4",
        "//src/rml/rmlv:soa",
        "//src/rml/rmlv:vec",
        ":interpolate",
        ],
    copts = RSR_DEFAULT_COPTS,
    )

cc_library(
    name = "interpolate",
    srcs = ["rglv_interpolate.cxx"],
    hdrs = ["rglv_interpolate.hxx"],
    deps = [
        "//src/rml/rmlv:soa"
        ],
    copts = RSR_DEFAULT_COPTS,
    )

cc_library(
    name = "triangle",
    srcs = ["rglv_triangle.cxx"],
    hdrs = ["rglv_triangle.hxx"],
    deps = [
        "//src/rml/rmlg:irect",
        "//src/rml/rmlv:mvec4",
        "//src/rml/rmlv:soa",
        "//src/rml/rmlv:vec",
        ":interpolate",
        ],
    copts = RSR_DEFAULT_COPTS,
    )

cc_test(
    name = "triangle_unittest",
    srcs = ["rglv_triangle.t.cxx"],
    deps = [
        "//src/rcl/rcls:aligned_containers",
        "//src/rgl/rglr:blend",
        "//src/rgl/rglr:canvas",
        "//src/rgl/rglr:canvas_util",
        "//src/rgl/rglr:texture",
        "//src/rml/rmlm:mat4",
        "//src/rml/rmlv:soa",
        "//src/rml/rmlv:vec",
        ":fragment",
        ":gpu",  # DefaultTarget
        ":interpolate",
        ":triangle",
        ],
    copts = RSR_DEFAULT_COPTS,
    )

cc_library(
    name = "gpu_shaders",
    srcs = ["rglv_gpu_shaders.cxx"],
    hdrs = ["rglv_gpu_shaders.hxx"],
    deps = [
        "//src/rml/rmlm:mat4",
        "//src/rml/rmlv:soa",
        "//src/rml/rmlv:vec",
        ":interpolate",
        ],
    copts = RSR_DEFAULT_COPTS,
    )
