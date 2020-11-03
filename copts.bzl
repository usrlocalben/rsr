MSVC_FLAGS = [
    "/EHsc",  # C++ exceptions: enabled
    "/GS-",  # security checks: disabled
    "/std:c++17",  # C++ standard: c++17
    "/permissive-",  # standard conformance: strict

    "/W4",
    "/WX",
    "/wd4324",  # structure padded due to alignment specifier
    #"/wd4305",  # "truncation from 'double' to 'float'
    #"/wd4244",  # "conversion from 'double' to 'float' possible loss of data
    #"/wd4005",  # macro redefinition
    #"/wd4267",  # conversion from 'size_t' to 'type', possible loss of data

    "/DUNICODE",
    "/D_UNICODE",
    "/D_ENABLE_EXTENDED_ALIGNED_STORAGE",
    "/DNOMINMAX",  # Don't define min and max macros (windows.h)
    ]

CLANG_CL_FLAGS = [
    "-msse4.1",
    "-Wno-unused-parameter",
    "-Wno-unused-variable",
    "-Wno-unused-function",
    "-Wno-builtin-macro-redefined",
    "-Wno-unused-command-line-argument",  # rocket .c sources don't use /std:c++17
    ]

MSVC_RELEASE_FLAGS = [
    "/fp:fast",  # fpu mode: less precise
    ]

RSR_DEFAULT_COPTS = MSVC_FLAGS + MSVC_RELEASE_FLAGS + \
                    select({"//:clang_cl_compiler": CLANG_CL_FLAGS,
                            "//conditions:default": []})
