set(TARGET_NAME usdtweak_widgets)
add_library(
  ${TARGET_NAME}
  Dockspace.cpp
  TextEditor.cpp
  TextEditor.h
  CompositionEditor.cpp
  CompositionEditor.h
  FileBrowser.cpp
  FileBrowser.h
  LayerEditor.cpp
  LayerEditor.h
  LayerHierarchyEditor.cpp
  LayerHierarchyEditor.h
  ModalDialogs.cpp
  ModalDialogs.h
  PrimSpecEditor.cpp
  PrimSpecEditor.h
  PropertyEditor.cpp
  PropertyEditor.h
  RendererSettings.cpp
  RendererSettings.h
  StageOutliner.cpp
  StageOutliner.h
  ContentBrowser.cpp
  ContentBrowser.h
  TextFilter.cpp
  TextFilter.h
  Timeline.cpp
  Timeline.h
  ValueEditor.cpp
  ValueEditor.h
  VariantEditor.cpp
  VariantEditor.h)
target_include_directories(${TARGET_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(${TARGET_NAME} PRIVATE usd usdtweak_stage imgui iconfontcppheaders fontawesome filesystem)
