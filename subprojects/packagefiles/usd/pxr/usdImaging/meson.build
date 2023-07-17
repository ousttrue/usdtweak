subdir('usdImaging')
subdir('usdImagingGL')
subdir('usdAppUtils')

usd_usdImaging_dep = declare_dependency(
    link_with: [
        usdImaging_lib,
        usdImagingGL_lib,
        usdAppUtils_lib,
    ],
    dependencies: [usd_dep, usd_imaging_dep],
)
