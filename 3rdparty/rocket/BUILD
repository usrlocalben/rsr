cxx_library(
    srcs=['rocket/device.c', 'rocket/track.c'],
    include_dir='.',
    deps=[':socketlib'],
    )

cxx_binary_library(
    name='socketlib',
    include_dir='include',
    lib_dir='lib\\x64',
    inputs=['ws2_32.lib'],
    )
