cpp = meson.get_compiler('cpp')

usdUtils_lib = shared_library(
    'usd_usdUtils',
    [
        # PUBLIC_CLASSES
        'authoring.cpp',
        'coalescingDiagnosticDelegate.cpp',
        'conditionalAbortDiagnosticDelegate.cpp',
        'debugCodes.cpp',
        'dependencies.cpp',
        'flattenLayerStack.cpp',
        'introspection.cpp',
        'pipeline.cpp',
        'registeredVariantSet.cpp',
        'sparseValueWriter.cpp',
        'stageCache.cpp',
        'stitch.cpp',
        'stitchClips.cpp',
        'timeCodeRange.cpp',
    ],
    install: true,
    implicit_include_directories: false,
    cpp_args: args + [
        '-DUSDUTILS_EXPORTS=1',
    ],
    dependencies: [usd_base_dep],
    link_with: [
        usd_lib,
        kind_lib,
        pcp_lib,
        sdf_lib,
        usdGeom_lib,
        usdShade_lib,
        ar_lib,
    ],
)
