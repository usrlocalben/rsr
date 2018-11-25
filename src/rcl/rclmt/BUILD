cxx_library(
    name='barrier',
    srcs=['rclmt_barrier.cxx'],
    hdrs=['rclmt_barrier.hxx'],
    )

cxx_library(
    name='jobsys',
    srcs=['rclmt_jobsys.cxx'],
    hdrs=['rclmt_jobsys.hxx'],
    deps=[
        'src/rcl/rcls:timer',
        'src/rcl/rclmt:barrier',
        ],
    )

cxx_test(
    name='jobsys_unittest',
    srcs=['rclmt_jobsys.t.cxx'],
    deps=[
        'src/rcl/rclmt:jobsys',
        'src/rcl/rcls:timer',
        ],
    )
