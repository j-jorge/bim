cmake_minimum_required(VERSION 3.22)

project(axmol)

if (CMAKE_VERSION VERSION_GREATER_EQUAL 3.31)
  # Normalize DESTINATION values given in the install() command.
  cmake_policy(SET CMP0177 NEW)
endif()

set(CMAKE_CXX_STANDARD 20)

execute_process(
  COMMAND iscool-shell-config --cmake-modules
  OUTPUT_VARIABLE iscool_cmake_modules
 )

if(NOT iscool_cmake_modules)
  message(FATAL_ERROR "Could not find directory for IsCool's CMake modules.")
endif()

string(STRIP "${iscool_cmake_modules}" iscool_cmake_modules)

set(CMAKE_MODULE_PATH
  ${CMAKE_MODULE_PATH}
  ${iscool_cmake_modules}
  ${axmol_root}/cmake/Modules
 )

include(unity-build)
include(GNUInstallDirs)

if (AX_ENABLE_TRACY)
  find_package(Tracy REQUIRED)
endif()

macro(assign_platform_variables)
  if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    set(target_linux 1)
    set(third_party_system_dir linux)
  elseif(CMAKE_SYSTEM_NAME STREQUAL "Android")
    set(target_android 1)
    set(third_party_system_dir "android/${CMAKE_ANDROID_ARCH_ABI}")
  endif()
endmacro()

macro(assign_path_variables)
  set(library_install_dir "${CMAKE_INSTALL_LIBDIR}")
  set(header_install_dir "${CMAKE_INSTALL_INCLUDEDIR}")
  set(resource_install_dir "${CMAKE_INSTALL_DATAROOTDIR}")

  set(source_root ${axmol_root}/core/)
  set(third_party_root ${axmol_root}/3rdparty/)
  set(third_party_binary_dir "${CMAKE_CURRENT_BINARY_DIR}/third-party-bin")
  set(extensions_root ${axmol_root}/extensions)

  # To match Axmol's CMakeLists.txt
  set(_AX_ROOT ${axmol_root})
  set(_AX_CXX_STD 20)
  set(_AX_ANDROID_PROJECT_DIR ".")
endmacro()

macro(include_axmol)
  set(AX_ENABLE_EXT_LUA OFF CACHE BOOL "" FORCE)
  set(AX_ENABLE_3D OFF CACHE BOOL "" FORCE)
  set(AX_ENABLE_PHYSICS3D OFF CACHE BOOL "" FORCE)
  set(AX_ENABLE_NAVMESH OFF CACHE BOOL "" FORCE)
  set(AX_ENABLE_MEDIA OFF CACHE BOOL "" FORCE)
  set(AX_ENABLE_AUDIO ON CACHE BOOL "" FORCE)

  # Axmol forces this path for Android builds. We use it for all
  # builds to simplify the later commands.
  set(AXSLCC_OUT_DIR ${CMAKE_CURRENT_BINARY_DIR}/build/runtime/axslc)

  include(${axmol_root}/cmake/Modules/AXLinkHelpers.cmake)
  include(${axmol_root}/cmake/Modules/AXBuildSet.cmake)
  include(${axmol_root}/cmake/Modules/AXConfigSIMD.cmake)
endmacro()

macro(include_platform_modules)
   if(target_linux)
     find_package(PkgConfig REQUIRED)
     pkg_check_modules(GTK3 REQUIRED gtk+-3.0)
   endif()
endmacro()

macro(configure)
  set(AX_BUILD_NUM "0")
  set(AX_GIT_PRESENT "0")
  set(AX_GIT_BRANCH "UNKNOWN")
  set(AX_GIT_COMMIT_HASH "unknown")
  configure_file(
    "${source_root}/axmolver.h.in"
    "${source_root}/axmolver.h"
  )
endmacro()

macro(include_third_parties)
  include(third-party/concurrentqueue.cmake)
  include(third-party/glad.cmake)
  include(third-party/jpeg.cmake)
  include(third-party/llhttp.cmake)
  include(third-party/openssl.cmake)
  include(third-party/poly2tri.cmake)
  include(third-party/robin-map.cmake)
  include(third-party/yasio.cmake)
  include(third-party/zlib.cmake)

  if (target_linux)
    include(third-party/glfw.cmake)
    set(platform_install_targets glfw)
  elseif(target_android)
    include(third-party/jni.cmake)
    set(ALSOFT_BACKEND_PIPEWIRE OFF CACHE BOOL "" FORCE)
  endif()

  set(AX_USE_ALSOFT ON CACHE BOOL "" FORCE)
  set(AX_USE_GL ${AX_USE_GL} CACHE BOOL "" FORCE)
  set(AX_GLES_PROFILE ${AX_GLES_PROFILE} CACHE STRING "" FORCE)

  if (AL_LIBTYPE_STATIC)
    set(AX_USE_ALSOFT_STATIC ON CACHE BOOL "" FORCE)
  endif()

  set(AX_WITH_ASTCENC ON CACHE BOOL "" FORCE)
  set(AX_WITH_BOX2D OFF CACHE BOOL "" FORCE)
  set(AX_WITH_BULLET OFF CACHE BOOL "" FORCE)
  set(AX_WITH_CARES OFF CACHE BOOL "" FORCE)
  set(AX_WITH_CHIPMUNK OFF CACHE BOOL "" FORCE)
  set(AX_WITH_CLIPPER2 ON CACHE BOOL "" FORCE)
  set(AX_WITH_CURL OFF CACHE BOOL "" FORCE)
  set(AX_WITH_FASTLZ OFF CACHE BOOL "" FORCE)
  set(AX_WITH_JPEG ON CACHE BOOL "" FORCE)
  set(AX_WITH_KCP OFF CACHE BOOL "" FORCE)
  set(AX_WITH_LZ4 OFF CACHE BOOL "" FORCE)
  set(AX_WITH_OBOE OFF CACHE BOOL "" FORCE)
  set(AX_WITH_POLY2TRI OFF CACHE BOOL "" FORCE)
  set(AX_WITH_RECAST OFF CACHE BOOL "" FORCE)
  set(AX_WITH_UNZIP ON CACHE BOOL "" FORCE)
  set(AX_WITH_WEBP OFF CACHE BOOL "" FORCE)
  set(AX_WITH_YAML_CPP OFF CACHE BOOL "" FORCE)

  add_subdirectory(
    "${third_party_root}"
    "${third_party_binary_dir}"
  )
endmacro()

macro(assign_common_sources)
  set(axmol_sources
    ${source_root}/axmol.cpp
    ${source_root}/2d/Action.cpp
    ${source_root}/2d/ActionCamera.cpp
    ${source_root}/2d/ActionCatmullRom.cpp
    ${source_root}/2d/ActionEase.cpp
    ${source_root}/2d/ActionGrid.cpp
    ${source_root}/2d/ActionGrid3D.cpp
    ${source_root}/2d/ActionInstant.cpp
    ${source_root}/2d/ActionInterval.cpp
    ${source_root}/2d/ActionManager.cpp
    ${source_root}/2d/ActionTiledGrid.cpp
    ${source_root}/2d/ActionTween.cpp
    ${source_root}/2d/Animation.cpp
    ${source_root}/2d/AnimationCache.cpp
    ${source_root}/2d/AtlasNode.cpp
    ${source_root}/2d/AutoPolygon.cpp
    ${source_root}/2d/Camera.cpp
    ${source_root}/2d/CameraBackgroundBrush.cpp
    ${source_root}/2d/ClippingNode.cpp
    ${source_root}/2d/ClippingRectangleNode.cpp
    ${source_root}/2d/Component.cpp
    ${source_root}/2d/ComponentContainer.cpp
    ${source_root}/2d/DrawNode.cpp
    ${source_root}/2d/Font.cpp
    ${source_root}/2d/FontAtlas.cpp
    ${source_root}/2d/FontAtlasCache.cpp
    ${source_root}/2d/FontCharMap.cpp
    ${source_root}/2d/FontFNT.cpp
    ${source_root}/2d/FontFreeType.cpp
    ${source_root}/2d/Grid.cpp
    ${source_root}/2d/Label.cpp
    ${source_root}/2d/LabelAtlas.cpp
    ${source_root}/2d/Layer.cpp
    ${source_root}/2d/Node.cpp
    ${source_root}/2d/NodeGrid.cpp
    ${source_root}/2d/ParticleBatchNode.cpp
    ${source_root}/2d/ParticleSystem.cpp
    ${source_root}/2d/ParticleSystemQuad.cpp
    ${source_root}/2d/PlistSpriteSheetLoader.cpp
    ${source_root}/2d/ProgressTimer.cpp
    ${source_root}/2d/ProtectedNode.cpp
    ${source_root}/2d/RenderTexture.cpp
    ${source_root}/2d/Scene.cpp
    ${source_root}/2d/Sprite.cpp
    ${source_root}/2d/SpriteBatchNode.cpp
    ${source_root}/2d/SpriteFrame.cpp
    ${source_root}/2d/SpriteFrameCache.cpp
    ${source_root}/2d/SpriteSheetLoader.cpp
    ${source_root}/2d/Transition.cpp
    ${source_root}/2d/TweenFunction.cpp
    ${source_root}/3d/3DProgramInfo.cpp
    ${source_root}/3d/Frustum.cpp
    ${source_root}/3d/Plane.cpp
    ${source_root}/audio/AudioCache.cpp
    ${source_root}/audio/AudioDecoder.cpp
    ${source_root}/audio/AudioDecoderMp3.cpp
    ${source_root}/audio/AudioDecoderWav.cpp
    ${source_root}/audio/AudioEffectsExtension.cpp
    ${source_root}/audio/AudioEngine.cpp
    ${source_root}/audio/AudioEngineImpl.cpp
    ${source_root}/audio/AudioPlayer.cpp
    ${source_root}/base/AsyncTaskPool.cpp
    ${source_root}/base/atitc.cpp
    ${source_root}/base/AutoreleasePool.cpp
    ${source_root}/base/base64.cpp
    ${source_root}/base/Configuration.cpp
    ${source_root}/base/Console.cpp
    ${source_root}/base/Data.cpp
    ${source_root}/base/Director.cpp
    ${source_root}/base/etc1.cpp
    ${source_root}/base/etc2.cpp
    ${source_root}/base/Event.cpp
    ${source_root}/base/EventCustom.cpp
    ${source_root}/base/EventDispatcher.cpp
    ${source_root}/base/EventFocus.cpp
    ${source_root}/base/EventKeyboard.cpp
    ${source_root}/base/EventListener.cpp
    ${source_root}/base/EventListenerAcceleration.cpp
    ${source_root}/base/EventListenerController.cpp
    ${source_root}/base/EventListenerCustom.cpp
    ${source_root}/base/EventListenerFocus.cpp
    ${source_root}/base/EventListenerKeyboard.cpp
    ${source_root}/base/EventListenerMouse.cpp
    ${source_root}/base/EventListenerTouch.cpp
    ${source_root}/base/EventMouse.cpp
    ${source_root}/base/EventTouch.cpp
    ${source_root}/base/FPSImages.cpp
    ${source_root}/base/IMEDispatcher.cpp
    ${source_root}/base/JobSystem.cpp
    ${source_root}/base/Logging.cpp
    ${source_root}/base/NinePatchImageParser.cpp
    ${source_root}/base/NS.cpp
    ${source_root}/base/Object.cpp
    ${source_root}/base/ObjectFactory.cpp
    ${source_root}/base/pvr.cpp
    ${source_root}/base/s3tc.cpp
    ${source_root}/base/Scheduler.cpp
    ${source_root}/base/ScriptSupport.cpp
    ${source_root}/base/StencilStateManager.cpp
    ${source_root}/base/TGAlib.cpp
    ${source_root}/base/Touch.cpp
    ${source_root}/base/Types.cpp
    ${source_root}/base/UserDefault.cpp
    ${source_root}/base/UTF8.cpp
    ${source_root}/base/Utils.cpp
    ${source_root}/base/Value.cpp
    ${source_root}/base/ZipUtils.cpp
    ${source_root}/math/AffineTransform.cpp
    ${source_root}/math/Color.cpp
    ${source_root}/math/Mat4.cpp
    ${source_root}/math/MathUtil.cpp
    ${source_root}/math/Quaternion.cpp
    ${source_root}/math/Rect.cpp
    ${source_root}/math/TransformUtils.cpp
    ${source_root}/math/Vec2.cpp
    ${source_root}/math/Vec3.cpp
    ${source_root}/math/Vec4.cpp
    ${source_root}/network/HttpClient.cpp
    ${source_root}/network/HttpCookie.cpp
    ${source_root}/network/Uri.cpp
    ${source_root}/platform/ApplicationBase.cpp
    ${source_root}/platform/FileStream.cpp
    ${source_root}/platform/FileUtils.cpp
    ${source_root}/platform/Image.cpp
    ${source_root}/platform/RenderView.cpp
    ${source_root}/platform/SAXParser.cpp
    ${source_root}/renderer/CallbackCommand.cpp
    ${source_root}/renderer/CustomCommand.cpp
    ${source_root}/renderer/GroupCommand.cpp
    ${source_root}/renderer/MeshCommand.cpp
    ${source_root}/renderer/Pass.cpp
    ${source_root}/renderer/QuadCommand.cpp
    ${source_root}/renderer/RenderCommand.cpp
    ${source_root}/renderer/RenderState.cpp
    ${source_root}/renderer/Renderer.cpp
    ${source_root}/renderer/Texture2D.cpp
    ${source_root}/renderer/TextureAtlas.cpp
    ${source_root}/renderer/TextureCache.cpp
    ${source_root}/renderer/TextureCube.cpp
    ${source_root}/renderer/TrianglesCommand.cpp
    ${source_root}/renderer/Shaders.cpp
    ${source_root}/renderer/backend/CommandBuffer.cpp
    ${source_root}/renderer/backend/DepthStencilState.cpp
    ${source_root}/renderer/backend/DriverBase.cpp
    ${source_root}/renderer/backend/PixelFormatUtils.cpp
    ${source_root}/renderer/backend/Program.cpp
    ${source_root}/renderer/backend/ProgramManager.cpp
    ${source_root}/renderer/backend/ProgramState.cpp
    ${source_root}/renderer/backend/ProgramStateRegistry.cpp
    ${source_root}/renderer/backend/ShaderCache.cpp
    ${source_root}/renderer/backend/ShaderModule.cpp
    ${source_root}/renderer/backend/Texture.cpp
    ${source_root}/renderer/backend/Types.cpp
    ${source_root}/renderer/backend/VertexLayout.cpp
    ${source_root}/renderer/backend/opengl/BufferGL.cpp
    ${source_root}/renderer/backend/opengl/CommandBufferGL.cpp
    ${source_root}/renderer/backend/opengl/CommandBufferGLES2.cpp
    ${source_root}/renderer/backend/opengl/DepthStencilStateGL.cpp
    ${source_root}/renderer/backend/opengl/DriverGL.cpp
    ${source_root}/renderer/backend/opengl/OpenGLState.cpp
    ${source_root}/renderer/backend/opengl/ProgramGL.cpp
    ${source_root}/renderer/backend/opengl/RenderPipelineGL.cpp
    ${source_root}/renderer/backend/opengl/RenderTargetGL.cpp
    ${source_root}/renderer/backend/opengl/ShaderModuleGL.cpp
    ${source_root}/renderer/backend/opengl/TextureGL.cpp
    ${source_root}/renderer/backend/opengl/UtilsGL.cpp
    ${source_root}/ui/UIHelper.cpp
    ${source_root}/ui/UILayout.cpp
    ${source_root}/ui/UILayoutComponent.cpp
    ${source_root}/ui/UILayoutManager.cpp
    ${source_root}/ui/UILayoutParameter.cpp
    ${source_root}/ui/UIListView.cpp
    ${source_root}/ui/UIPageView.cpp
    ${source_root}/ui/UIPageViewIndicator.cpp
    ${source_root}/ui/UIRichText.cpp
    ${source_root}/ui/UIScale9Sprite.cpp
    ${source_root}/ui/UIScrollView.cpp
    ${source_root}/ui/UIScrollViewBar.cpp
    ${source_root}/ui/UIWidget.cpp
    ${source_root}/ui/UIEditBox/UIEditBox.cpp
    ${source_root}/ui/UIEditBox/UIEditBoxImpl-common.cpp

    ${extensions_root}/GUI/src/GUI/ScrollView/ScrollView.cpp
    ${extensions_root}/GUI/src/GUI/ScrollView/TableView.cpp
    ${extensions_root}/GUI/src/GUI/ScrollView/TableViewCell.cpp

    ${poly2tri_sources}
   )

  set(
    axmol_individual_sources
    ${source_root}/audio/AudioDecoderManager.cpp
    ${source_root}/audio/AudioDecoderVorbis.cpp
    ${source_root}/base/astc.cpp
   )
endmacro()

macro(assign_platform_sources)
  if(target_linux)
    set(
      platform_sources
      ${source_root}/platform/linux/Application-linux.cpp
      ${source_root}/platform/linux/Common-linux.cpp
      ${source_root}/platform/linux/FileUtils-linux.cpp
      ${source_root}/ui/UIEditBox/UIEditBoxImpl-linux.cpp
    )
    set(
      axmol_platform_individual_sources
      ${source_root}/platform/linux/Device-linux.cpp
      ${source_root}/platform/RenderViewImpl.cpp
    )
  elseif(target_android)
    set(
      platform_sources
      ${source_root}/platform/android/Application-android.cpp
      ${source_root}/platform/android/Common-android.cpp
      ${source_root}/platform/android/Device-android.cpp
      ${source_root}/platform/android/FileUtils-android.cpp
      ${source_root}/platform/android/javaactivity-android.cpp
      ${source_root}/platform/android/RenderViewImpl-android.cpp
      ${source_root}/platform/android/jni/JniHelper.cpp
      ${source_root}/platform/android/jni/Java_dev_axmol_lib_AxmolEngine.cpp
      ${source_root}/platform/android/jni/Java_dev_axmol_lib_AxmolRenderer.cpp
      ${source_root}/platform/android/jni/TouchesJni.cpp
      ${source_root}/ui/UIEditBox/UIEditBoxImpl-android.cpp
    )

    set(
      axmol_platform_individual_sources
      ${CMAKE_ANDROID_NDK}/sources/android/cpufeatures/cpu-features.c
    )
  endif()

  set(axmol_sources ${axmol_sources} ${platform_sources})
endmacro()

macro(set_include_directories target)
  if(target_linux)
    set(
      platform_include_dirs
      ${GTK3_INCLUDE_DIRS}
    )
  elseif(target_android)
    set(
      platform_include_dirs
      ${CMAKE_ANDROID_NDK}/sources/android/cpufeatures
    )
  endif()

  target_include_directories(
    ${target}
    PUBLIC
    ${axmol_root}
    ${source_root}
    ${source_root}/platform

    ${platform_include_dirs}
    ${third_party_root}
  )
endmacro()

macro(set_link_libraries target)
  target_link_libraries(
    ${target}
    PRIVATE
    3rdparty
  )
endmacro()

macro(set_compile_definitions target tracy)
  target_compile_definitions(
    ${target}
    PUBLIC
    AX_USE_PHYSICS=${AX_USE_PHYSICS}
    AX_USE_3D_PHYSICS=${AX_USE_3D_PHYSICS}
    AX_USE_GL=${AX_USE_GL}
    AX_USE_NAVMESH=${AX_USE_NAVMESH}
    AX_USE_TIFF=${AX_USE_TIFF}
    AX_USE_WEBP=${AX_USE_WEBP}
    AX_USE_WIC=${AX_USE_WIC}
    AX_ENABLE_SCRIPT_BINDING=${AX_ENABLE_SCRIPT_BINDING}
    AX_ENABLE_PREMULTIPLIED_ALPHA=${AX_ENABLE_PREMULTIPLIED_ALPHA}
    AX_ENABLE_TRACY=${tracy}

    PRIVATE
    P2T_STATIC_EXPORTS
   )

  if(LINUX)
    target_compile_definitions(${target} PUBLIC LINUX)
  endif()
endmacro()

macro(compile_shaders)
  ax_target_compile_shaders(
    axmol
    FILES
    ${source_root}/renderer/shaders/label_normal.frag
    ${source_root}/renderer/shaders/label_outline.frag
    ${source_root}/renderer/shaders/positionColor.frag
    ${source_root}/renderer/shaders/positionColorLengthTexture.frag
    ${source_root}/renderer/shaders/positionColorLengthTexture.vert
    ${source_root}/renderer/shaders/positionColorTextureAsPointsize.vert
    ${source_root}/renderer/shaders/positionTextureColor.frag
    ${source_root}/renderer/shaders/positionTextureColor.vert
    ${source_root}/renderer/shaders/positionTextureColorAlphaTest.frag
    ${source_root}/renderer/shaders/positionUColor.vert
  )
endmacro()

macro(declare_library)
  include_third_parties()
  assign_common_sources()
  assign_platform_sources()

  set(unit_extension cpp)

  make_compilation_unit(
    unity_build_file
    axmol
    ${unit_extension}
    ${axmol_sources}
   )

   add_library(
     axmol
     ${unity_build_file}
     ${axmol_individual_sources}
     ${axmol_platform_individual_sources}
   )

  set_include_directories(axmol)
  set_link_libraries(axmol)
  set_compile_definitions(axmol 0)

   if (AX_ENABLE_TRACY)
     add_library(
       axmol_tracy
       ${unity_build_file}
       ${axmol_individual_sources}
       ${axmol_platform_individual_sources}
     )

     set_include_directories(axmol_tracy)
     set_link_libraries(axmol_tracy)
     set_compile_definitions(axmol_tracy 1)

     target_link_libraries(
       axmol_tracy
       PRIVATE
       Tracy::TracyClient
     )

   endif()

  compile_shaders()
endmacro()

function(install_headers root target)
  file(GLOB_RECURSE headers RELATIVE ${root} ${root}/*.h ${root}/*.inl)

  foreach(header ${headers})
    get_filename_component(header_path ${header} DIRECTORY)
    install(
      FILES ${root}/${header}
      DESTINATION ${header_install_dir}/axmol/${target}/${header_path}/
     )
  endforeach()
endfunction()

function(install_shaders root target)
  install(
    DIRECTORY ${AXSLCC_OUT_DIR}/
    DESTINATION ${resource_install_dir}/axmol/shaders/
  )
endfunction()

macro(install_products)
  install_headers(${source_root} .)
  install_headers(${extensions_root}/GUI/src/ extensions)
  install(
    FILES
    ${extensions_root}/ExtensionExport.h
    ${extensions_root}/ExtensionMacros.h
    DESTINATION ${header_install_dir}/axmol/extensions/
  )

  install_shaders(${source_root} .)
  install(TARGETS axmol DESTINATION ${library_install_dir})

  if (AX_ENABLE_TRACY)
    install(TARGETS axmol_tracy DESTINATION ${library_install_dir})
  endif()

  install(TARGETS
    ${platform_install_targets}
    astcenc
    clipper2
    ConvertUTF
    freetype
    glad
    llhttp
    ogg
    OpenAL
    png
    pugixml
    simdjson
    unzip
    xxhash
    yasio
    DESTINATION ${library_install_dir}
  )
endmacro()

assign_platform_variables()
assign_path_variables()
include_axmol()
include_platform_modules()

configure()

declare_library()
install_products()
