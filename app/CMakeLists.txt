add_library(usdtweak "" main.cpp window.cpp)
target_link_libraries(usdtweak PRIVATE glfw usd usdtweak_lib imgui)
