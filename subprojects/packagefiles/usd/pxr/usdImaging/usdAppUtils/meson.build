cpp = meson.get_compiler('cpp')

usdAppUtils_lib = shared_library(
    'usd_usdAppUtils',
    [
        # PUBLIC_CLASSES
        'camera.cpp',
        'frameRecorder.cpp',
    ],
    install: true,
    implicit_include_directories: false,
    cpp_args: args + [
        '-DUSDAPPUTILS_EXPORTS=1',
    ],
    dependencies: [usd_dep, usd_imaging_dep],
    link_with: [
        usdImaging_lib,
        usdImagingGL_lib,
    ],
)
