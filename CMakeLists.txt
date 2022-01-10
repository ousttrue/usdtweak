cmake_minimum_required(VERSION 3.11)
project(UsdTweak)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR}/Debug/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR}/Debug/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR}/Debug/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}/Release/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}/Release/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}/Release/bin)

set(CMAKE_CXX_STANDARD 14)
add_compile_options("$<$<C_COMPILER_ID:MSVC>:/utf-8>")
add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/utf-8>")

# glfw
set(GLFW_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../glfw)
add_library(glfw INTERFACE)
target_include_directories(glfw INTERFACE ${GLFW_DIR}/include)
target_link_libraries(
  glfw
  INTERFACE
    "$<$<CONFIG:Debug>:${CMAKE_CURRENT_SOURCE_DIR}/../../build/Debug/lib/glfw.lib>"
    "$<$<CONFIG:Release>:${CMAKE_CURRENT_SOURCE_DIR}/../../build/Release/glfw.lib>"
)

# boost & usd
set(DEBUG_DIR ${CMAKE_CURRENT_LIST_DIR}/../../build_debug)
set(RELEASE_DIR ${CMAKE_CURRENT_LIST_DIR}/../../build_relase)

add_library(boost INTERFACE)
target_include_directories(
  boost INTERFACE "$<$<CONFIG:Debug>:${DEBUG_DIR}/include/boost-1_77>"
                  "$<$<CONFIG:Release>:${RELEASE_DIR}/include/boost-1_77>")

add_library(usd INTERFACE)
target_include_directories(
  usd INTERFACE "$<$<CONFIG:Debug>:${DEBUG_DIR}/include>"
                "$<$<CONFIG:Release>:${RELEASE_DIR}/include>")
target_compile_definitions(usd INTERFACE NOMINMAX)
target_link_libraries(
  usd
  INTERFACE boost
            usd_ar.lib
            usd_arch.lib
            usd_cameraUtil.lib
            usd_garch.lib
            usd_gf.lib
            usd_glf.lib
            usd_hd.lib
            usd_hdSt.lib
            usd_hdx.lib
            usd_hf.lib
            usd_hgi.lib
            usd_hgiGL.lib
            usd_hgiInterop.lib
            usd_hio.lib
            usd_js.lib
            usd_kind.lib
            usd_ndr.lib
            usd_pcp.lib
            usd_plug.lib
            usd_pxOsd.lib
            usd_sdf.lib
            usd_sdr.lib
            usd_tf.lib
            usd_trace.lib
            usd_usd.lib
            usd_usdAppUtils.lib
            usd_usdGeom.lib
            usd_usdHydra.lib
            usd_usdImaging.lib
            usd_usdImagingGL.lib
            usd_usdLux.lib
            usd_usdMedia.lib
            usd_usdPhysics.lib
            usd_usdRender.lib
            usd_usdRi.lib
            usd_usdRiImaging.lib
            usd_usdShade.lib
            usd_usdSkel.lib
            usd_usdSkelImaging.lib
            usd_usdUI.lib
            usd_usdUtils.lib
            usd_usdVol.lib
            usd_usdVolImaging.lib
            usd_vt.lib
            usd_work.lib)
target_link_directories(usd INTERFACE "$<$<CONFIG:Debug>:${DEBUG_DIR}/lib>"
                        "$<$<CONFIG:Release>:${RELEASE_DIR}/lib>")
# Remove warnings coming from usd and enable default multithreaded compilation
# on windows
target_compile_options(
  usd
  INTERFACE $<$<CXX_COMPILER_ID:MSVC>:/MP
            /wd4244
            /wd4305
            /wd4996
            /wd4506
            /wd4275>
            $<$<CXX_COMPILER_ID:GNU>:-Wno-deprecated>)

# usdtweak
subdirs(src)
