subdirs(3rdparty stage viewport)

add_library(
  usdtweak_lib
  Gui.h
  Dockspace.cpp
  widgets/TextEditor.cpp
  widgets/TextEditor.h
  widgets/CompositionEditor.cpp
  widgets/CompositionEditor.h
  widgets/FileBrowser.cpp
  widgets/FileBrowser.h
  widgets/LayerEditor.cpp
  widgets/LayerEditor.h
  widgets/LayerHierarchyEditor.cpp
  widgets/LayerHierarchyEditor.h
  widgets/ModalDialogs.cpp
  widgets/ModalDialogs.h
  widgets/PrimSpecEditor.cpp
  widgets/PrimSpecEditor.h
  widgets/PropertyEditor.cpp
  widgets/PropertyEditor.h
  widgets/RendererSettings.cpp
  widgets/RendererSettings.h
  widgets/StageOutliner.cpp
  widgets/StageOutliner.h
  widgets/ContentBrowser.cpp
  widgets/ContentBrowser.h
  widgets/TextFilter.cpp
  widgets/TextFilter.h
  widgets/Timeline.cpp
  widgets/Timeline.h
  widgets/ValueEditor.cpp
  widgets/ValueEditor.h
  widgets/VariantEditor.cpp
  widgets/VariantEditor.h)
target_include_directories(usdtweak_lib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(usdtweak_lib PUBLIC imgui usd usdtweak_stage usdtweak_hydra filesystem fontawesome
                                          iconfontcppheaders)
