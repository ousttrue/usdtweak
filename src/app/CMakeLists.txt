add_executable(usdtweak "" main.cpp window.cpp CommandLineOptions.h
                        CommandLineOptions.cpp)
target_link_libraries(usdtweak PRIVATE glfw usd usdtweak_stage usdtweak_widgets
                                       usdtweak_hydra imgui iconfontcppheaders)
