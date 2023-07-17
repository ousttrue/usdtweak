imgui_inc = include_directories('.')
imgui_lib = static_library(
    'imgui',
    ['imgui.cpp'],
    include_directories: imgui_inc,
)
imgui_dep = declare_dependency(
    include_directories: imgui_inc,
    link_with: imgui_lib,
)
