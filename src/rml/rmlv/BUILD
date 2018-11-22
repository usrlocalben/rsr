cxx_library(
    name='vec',
    srcs=['rmlv_vec.cxx'],
    hdrs=['rmlv_vec.hxx'],
    deps=[
        '3rdparty/fmt',
        ],
    )

cxx_library(
    name='mvec4',
    srcs=['rmlv_mvec4.cxx'],
    hdrs=['rmlv_mvec4.hxx'],
    deps=[
        '3rdparty/sse-pow',
        'src/rml/rmlv:mmutil',
        'src/rml/rmlv:vec',
        ],
    )

cxx_library(
    name='mmutil',
    srcs=['rmlv_mmutil.cxx'],
    hdrs=['rmlv_mmutil.hxx'],
    )

cxx_library(
    name='soa',
    srcs=['rmlv_soa.cxx'],
    hdrs=['rmlv_soa.hxx'],
    deps=[
        'src/rml/rmlv:mvec4',
        ],
    )

cxx_test(
    name='vec_test',
    srcs=['rmlv_vec.t.cxx'],
    deps=[
        '3rdparty/googletest:gtest_main',
        ':vec',
        ],
    )

cxx_test(
    name='soa_unittest',
    srcs=['rmlv_soa.t.cxx'],
    deps=[
        '3rdparty/googletest:gtest_main',
        ':soa',
        ':vec',
        ],
    )
