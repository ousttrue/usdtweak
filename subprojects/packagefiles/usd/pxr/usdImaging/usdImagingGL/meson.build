cpp = meson.get_compiler('cpp')

usdImagingGL_lib = shared_library(
    'usd_usdImagingGL',
    [
        # PUBLIC_CLASSES
        'engine.cpp',
        'renderParams.cpp',
    ],
    install: true,
    implicit_include_directories: false,
    cpp_args: args + ['-DUSDIMAGINGGL_EXPORTS=1'],
    dependencies: [usd_dep, usd_imaging_dep],
    link_with: [usdImaging_lib, glf_lib, hdx_lib, hd_lib, hgiGL_lib],
)
