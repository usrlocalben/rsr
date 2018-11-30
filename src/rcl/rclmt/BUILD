package(default_visibility = ["//visibility:public"])

cc_library(
    name = "barrier",
    srcs = ["rclmt_barrier.cxx"],
    hdrs = ["rclmt_barrier.hxx"],)

cc_library(
    name='jobsys',
    srcs=['rclmt_jobsys.cxx'],
    hdrs=['rclmt_jobsys.hxx'],
    deps=[
        '//src/rcl/rcls:timer',
        '//src/rcl/rclmt:barrier', ],)

cc_test(
    name='jobsys_unittest',
    srcs=['rclmt_jobsys.t.cxx'],
    deps=[
        '//src/rcl/rclmt:jobsys',
        '//src/rcl/rcls:timer', ],)
