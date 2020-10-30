package(default_visibility = ["//visibility:public"])

platform(
    name="x64_windows-clang-cl",
    constraint_values=[
        "@platforms//cpu:x86_64",
        "@platforms//os:windows",
        "@bazel_tools//tools/cpp:clang-cl",],)

config_setting(
    name = "clang_cl_compiler",
    flag_values={
        "@bazel_tools//tools/cpp:compiler": "clang-cl",},
    visibility = [":__subpackages__"],)
