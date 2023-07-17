subdir('3rdparty')

glfw_dep = dependency('glfw3')

executable(
    'usdtweak',
    [
        'main.cpp',
    ],
    install: true,
    include_directories: include_directories(
        'viewport',
        'commands',
        'resources',
    ),
    dependencies: [usd_dep, glfw_dep, imgui_dep, third_dep],
)
