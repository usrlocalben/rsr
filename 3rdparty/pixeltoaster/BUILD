cxx_library(
    # name='pixeltoaster',
    srcs=['PixelToaster.cpp'],
    include_dir='',
    # hdrs=['PixelToaster.h'],
    # private_hdrs=[
    #     'PixelToasterCommon.h',
    #   'PixelToasterConversion.h',
    #  'PixelToasterWindows.h'],
    deps=['3rdparty/directx'],
)

cxx_exe(
    name='example_floating_point',
    srcs=['ExampleFloatingPoint.cpp'],
    deps=[':pixeltoaster'],
)
