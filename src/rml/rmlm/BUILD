package(default_visibility = ["//visibility:public"])

cc_library(
    name='mat4',
    srcs=['rmlm_mat4.cxx'],
    hdrs=['rmlm_mat4.hxx'],
    deps=[
        '//3rdparty/fmt',
        '//src/rml/rmlv:vec',
        '//src/rml/rmlv:soa', ],)

cc_test(
    name='mat4_unittest',
    srcs=['rmlm_mat4.t.cxx'],
    deps=[
        '//3rdparty/googletest:gtest_main',
        '//src/rml/rmlv:vec',
        ':mat4', ],)
