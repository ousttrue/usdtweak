cpp = meson.get_compiler('cpp')

plug_lib = shared_library(
    'usd_plug',
    [
        # PUBLIC_CLASSES
        'interfaceFactory.cpp',
        'notice.cpp',
        'plugin.cpp',
        'registry.cpp',
        'staticInterface.cpp',

        # PRIVATE_CLASSES
        'debugCodes.cpp',
        'info.cpp',
        'testPlugBase.cpp',

        # CPPFILES
        'initConfig.cpp',

    ],
    install: true,
    implicit_include_directories: false,
    include_directories: pxr_inc,

    cpp_args: args + [
        '-DPLUG_EXPORTS=1',
        '-DPXR_BUILD_LOCATION=pluginfo',
        '-DPXR_PLUGIN_BUILD_LOCATION=../plugin/pluginfo',
    ],
    dependencies: [boost_dep, tbb_dep],
    link_with: [arch_lib, tf_lib, js_lib, trace_lib, work_lib],
)

install_data('plugInfo.json', install_dir: 'bin/pluginfo')
install_data('plugInfo.json', install_dir: 'plugin/pluginfo')
