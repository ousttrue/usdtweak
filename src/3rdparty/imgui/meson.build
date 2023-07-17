imgui_inc = include_directories('.')
imgui_lib = static_library(
    'imgui',
    [
        'imgui.cpp',
        'imgui_draw.cpp',
        'imgui_tables.cpp',
        'imgui_widgets.cpp',
        'imgui_stdlib.cpp',
        'imgui_impl_glfw.cpp',
        'imgui_impl_opengl3.cpp',
    ],
    include_directories: imgui_inc,
    dependencies: [glfw_dep],
)
imgui_dep = declare_dependency(
    include_directories: imgui_inc,
    link_with: imgui_lib,
)
