MSVC_FLAGS = [
    "/EHsc",  # C++ exceptions: enabled
    "/GS-",  # security checks: disabled
    "/std:c++17",  # C++ standard: c++17
    "/permissive-",  # standard conformance: strict

    "/W4",
    "/WX",
    #"/wd4305",  # "truncation from 'double' to 'float'
    #"/wd4244",  # "conversion from 'double' to 'float' possible loss of data
    #"/wd4005",  # macro redefinition
    #"/wd4267",  # conversion from 'size_t' to 'type', possible loss of data

    # for clang-cl
    "-Wno-builtin-macro-redefined",

    "/DUNICODE",
    "/D_UNICODE",
    "/D_ENABLE_EXTENDED_ALIGNED_STORAGE",
    "/DNOMINMAX",  # Don't define min and max macros (windows.h)

    #"/DENABLE_MUSIC",
    #"/DSYNC_PLAYER",
    ]

MSVC_RELEASE_FLAGS = [
    "/fp:fast",  # fpu mode: less precise
    ]

RSR_DEFAULT_COPTS = MSVC_FLAGS + MSVC_RELEASE_FLAGS
