load("//:copts.bzl", "RSR_DEFAULT_COPTS")

cc_library(
    name='gtest',
    srcs=[
        'src/gtest.cc',
        'src/gtest-death-test.cc',
        'src/gtest-filepath.cc',
        'src/gtest-matchers.cc',
        'src/gtest-port.cc',
        'src/gtest-printers.cc',
        'src/gtest-test-part.cc',
        'src/gtest-typed-test.cc',
        ],
    include_dir='include',
    copts = RSR_DEFAULT_COPTS,
    )

cc_library(
    name='gtest_main',
    srcs=['src/gtest_main.cc'],
    deps=[':gtest'],
    copts = RSR_DEFAULT_COPTS,
    )
