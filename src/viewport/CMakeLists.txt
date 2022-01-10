set(TARGET_NAME usdtweak_hydra)
add_library(
  ${TARGET_NAME}
  GridVert.cpp
  GridFrag.cpp
  Grid.cpp
  Grid.h
  manipulators/CameraRig.cpp
  manipulators/CameraRig.h
  manipulators/ManipulatorVert.cpp
  manipulators/ManipulatorFrag.cpp
  manipulators/CameraManipulator.cpp
  manipulators/CameraManipulator.h
  manipulators/Manipulator.cpp
  manipulators/Manipulator.h
  manipulators/MouseHoverManipulator.cpp
  manipulators/MouseHoverManipulator.h
  manipulators/PositionManipulator.cpp
  manipulators/PositionManipulator.h
  manipulators/RotationManipulator.cpp
  manipulators/RotationManipulator.h
  manipulators/ScaleManipulator.cpp
  manipulators/ScaleManipulator.h
  manipulators/SelectionManipulator.cpp
  manipulators/SelectionManipulator.h
  Viewport.cpp
  Viewport.h)
target_include_directories(${TARGET_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(${TARGET_NAME} PRIVATE usdtweak_stage imgui iconfontcppheaders)
