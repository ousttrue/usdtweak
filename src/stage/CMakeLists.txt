set(TARGET_NAME usdtweak_stage)
add_library(
  ${TARGET_NAME}
  StageLoader.cpp
  Selection.cpp
  AovUtil.cpp
  commands/Commands.cpp
  commands/Commands.h
  commands/Shortcuts.h
  commands/SdfCommandGroup.cpp
  commands/SdfCommandGroup.h
  commands/SdfLayerInstructions.cpp
  commands/SdfLayerInstructions.h
  commands/SdfUndoRecorder.cpp
  commands/SdfUndoRecorder.h
  commands/UndoLayerStateDelegate.cpp
  commands/UndoLayerStateDelegate.h)
target_include_directories(${TARGET_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_options(${TARGET_NAME} PRIVATE)
target_link_libraries(${TARGET_NAME} PUBLIC usd)
