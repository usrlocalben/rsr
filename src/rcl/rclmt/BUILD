load("//:copts.bzl", "RSR_DEFAULT_COPTS")

package(default_visibility = ["//visibility:public"])

cc_library(
    name = "barrier",
    srcs = ["rclmt_barrier.cxx"],
    hdrs = ["rclmt_barrier.hxx"],
    copts = RSR_DEFAULT_COPTS,
    )

cc_library(
    name = "jobsys",
    srcs = ["rclmt_jobsys.cxx"],
    hdrs = ["rclmt_jobsys.hxx"],
    deps = [
        "//src/rcl/rcls:timer",
        "//src/rcl/rclmt:barrier",
        ],
    copts = RSR_DEFAULT_COPTS,
    )

cc_test(
    name = "jobsys_unittest",
    srcs = ["rclmt_jobsys.t.cxx"],
    deps = [
        "//src/rcl/rclmt:jobsys",
        "//src/rcl/rcls:timer",
        ],
    copts = RSR_DEFAULT_COPTS,
    )

cc_library(
    name = "deferred",
    srcs = ["rclmt_deferred.cxx"],
    hdrs = ["rclmt_deferred.hxx"],
    copts = RSR_DEFAULT_COPTS,
    )

cc_library(
    name = "event",
    srcs = ["rclmt_event.cxx"],
    hdrs = ["rclmt_event.hxx"],
    deps = [
        "//3rdparty/fmt",
        ],
    copts = RSR_DEFAULT_COPTS,
    )

cc_library(
    name = "reactor",
    srcs = ["rclmt_reactor.cxx"],
    hdrs = ["rclmt_reactor.hxx"],
    deps = [
        "//3rdparty/fmt",
        ":event",
        ],
    copts = RSR_DEFAULT_COPTS,
    )

cc_library(
    name = "reactor_file",
    srcs = ["rclmt_reactor_file.cxx"],
    hdrs = ["rclmt_reactor_file.hxx"],
    deps = [
        "//3rdparty/fmt",
        "//src/rcl/rclt:util",
        "//src/rcl/rclw:winfile",
        ":deferred",
        ":event",
        ":reactor",
        ],
    copts = RSR_DEFAULT_COPTS,
    )

cc_library(
    name = "reactor_timer",
    srcs = ["rclmt_reactor_timer.cxx"],
    hdrs = ["rclmt_reactor_timer.hxx"],
    deps = [
        "//3rdparty/fmt",
        ":reactor",
        ":event",
        ],
    copts = RSR_DEFAULT_COPTS,
    )

cc_library(
    name = "reactor_keyboard",
    srcs = ["rclmt_reactor_keyboard.cxx"],
    hdrs = ["rclmt_reactor_keyboard.hxx"],
    deps = [
        ":reactor",
        ":event",
        ],
    copts = RSR_DEFAULT_COPTS,
    )


cc_binary(
    name = "reactor_timer_test",
    srcs = ["rclmt_reactor_timer.t.cxx"],
    deps = [
        "//3rdparty/fmt",
        "//src/rcl/rcls:timer",
        ":reactor",
        ":reactor_timer", ],
    copts = RSR_DEFAULT_COPTS,)
