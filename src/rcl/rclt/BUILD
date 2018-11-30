package(default_visibility = ["//visibility:public"])

cc_library(
    name='util',
    srcs=['rclt_util.cxx'],
    hdrs=['rclt_util.hxx'],)

cc_test(
    name='util_unittest',
    srcs=['rclt_util.t.cxx'],
    deps=[
        '//3rdparty/googletest:gtest_main',
        ':util', ],)
