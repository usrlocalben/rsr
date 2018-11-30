package(default_visibility = ["//visibility:public"])

cc_library(
    name='blend',
    srcs=['rglr_blend.cxx', 'rglr_blend.hxx'],
    hdrs=['rglr_blend.hxx'],
    deps=[
        '//src/rml/rmlv:soa',
        '//src/rml/rmlv:mvec4', ],)

cc_library(
    name='canvas',
    srcs=['rglr_canvas.cxx', 'rglr_canvas.hxx'],
    hdrs=['rglr_canvas.hxx'],
    deps=[
        '//3rdparty/pixeltoaster',
        '//src/rcl/rcls:aligned_containers',
        '//src/rml/rmlg:irect',
        '//src/rml/rmlv:soa',
        '//src/rml/rmlv:vec', ],)

cc_library(
    name='canvas_util',
    srcs=['rglr_canvas_util.cxx', 'rglr_canvas_util.hxx'],
    hdrs=['rglr_canvas_util.hxx'],
    deps=[
        '//3rdparty/pixeltoaster',
        '//3rdparty/ryg-srgb',
        '//src/rml/rmlg:irect',
        '//src/rml/rmlv:soa',
        ':canvas', ],)

cc_library(
    name='display_mode',
    srcs=['rglr_display_mode.cxx', 'rglr_display_mode.hxx'],
    hdrs=['rglr_display_mode.hxx'],)

cc_library(
    name='kawase',
    srcs=['rglr_kawase.cxx', 'rglr_kawase.hxx'],
    hdrs=['rglr_kawase.hxx'],
    deps=[
        '//src/rml/rmlv:mvec4',
        ':canvas', ],)

cc_library(
    name='pixeltoaster_util',
    srcs=['rglr_pixeltoaster_util.cxx', 'rglr_pixeltoaster_util.hxx'],
    hdrs=['rglr_pixeltoaster_util.hxx'],
    deps=[
        '//3rdparty/pixeltoaster',
        ':canvas', ],)

cc_library(
    name='profont',
    srcs=['rglr_profont.cxx', 'rglr_profont.hxx'],
    hdrs=['rglr_profont.hxx'],
    deps=[
        '//3rdparty/picopng',
        '//3rdparty/pixeltoaster',
        '//src/rcl/rclr:algorithm',
        '//src/rml/rmlv:vec',
        ':canvas', ],)

cc_library(
    name='texture',
    srcs=['rglr_texture.cxx', 'rglr_texture.hxx'],
    hdrs=['rglr_texture.hxx'],
    deps=[
        '//3rdparty/pixeltoaster',
        '//src/rcl/rcls:aligned_containers',
        '//src/rml/rmlg:pow2',
        '//src/rml/rmlv:mvec4',
        '//src/rml/rmlv:vec', ],)

cc_library(
    name='texture_store',
    srcs=['rglr_texture_store.cxx', 'rglr_texture_store.hxx'],
    hdrs=['rglr_texture_store.hxx'],
    deps=[
        '//3rdparty/fmt',
        '//src/rcl/rcls:file',
        ':texture',
        ':texture_load', ],)

cc_library(
    name='texture_sampler',
    srcs=['rglr_texture_sampler.cxx', 'rglr_texture_sampler.hxx'],
    hdrs=['rglr_texture_sampler.hxx'],
    deps=[
        '//3rdparty/pixeltoaster',
        '//src/rml/rmlg:pow2',
        '//src/rml/rmlv:mvec4',
        '//src/rml/rmlv:soa',
        '//src/rml/rmlv:vec', ],)

cc_library(
    name='texture_load',
    srcs=['rglr_texture_load.cxx', 'rglr_texture_load.hxx'],
    hdrs=['rglr_texture_load.hxx'],
    deps=[
        '//3rdparty/fmt',
        '//3rdparty/picopng',
        '//3rdparty/pixeltoaster',
        '//3rdparty/ryg-srgb',
        '//src/rcl/rcls:aligned_containers',
        '//src/rcl/rcls:file',
        '//src/rml/rmlg:pow2',
        '//src/rml/rmlv:mvec4',
        '//src/rml/rmlv:vec', ],)
