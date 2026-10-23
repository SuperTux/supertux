set(VITA_APP_NAME "SuperTux")
set(VITA_TITLEID "STUX00001")
set(VITA_VERSION "01.00")

set(ENABLE_OPENGL OFF CACHE BOOL "" FORCE)
set(ENABLE_NETWORKING OFF CACHE BOOL "" FORCE)
set(ENABLE_DISCORD OFF CACHE BOOL "" FORCE)
set(HIDE_NONMOBILE_OPTIONS ON CACHE BOOL "" FORCE)
set(USE_STATIC_SIMPLESQUIRREL ON CACHE BOOL "" FORCE)
set(BUILD_TESTING OFF CACHE BOOL "" FORCE)
set(SQ_DISABLE_INSTALLER ON CACHE BOOL "" FORCE)
set(SQ_DISABLE_INTERPRETER ON CACHE BOOL "" FORCE)
set(SSQ_BUILD_INSTALL OFF CACHE BOOL "" FORCE)

set_property(GLOBAL PROPERTY TARGET_SUPPORTS_SHARED_LIBS TRUE)

add_compile_definitions(_GNU_SOURCE)
string(APPEND CMAKE_C_FLAGS " -Wno-error=implicit-function-declaration")

set(VITA_LIBDIR "${VITASDK}/arm-vita-eabi/lib")
set(VITA_INCDIR "${VITASDK}/arm-vita-eabi/include")

function(vita_import name)
  add_library(${name} INTERFACE IMPORTED GLOBAL)
  if(ARGN)
    set_target_properties(${name} PROPERTIES INTERFACE_INCLUDE_DIRECTORIES "${ARGN}")
  endif()
endfunction()

vita_import(ZLIB)
vita_import(PNG)
vita_import(Freetype "${VITA_INCDIR}/freetype2")
vita_import(fmt)
vita_import(glm)
vita_import(PhysFS)
vita_import(SDL3)
vita_import(SDL3_image)
vita_import(SDL3_ttf)
vita_import(OpenAL "${VITA_INCDIR}/AL")
vita_import(Ogg)
vita_import(Vorbis)
vita_import(VorbisFile)

set(VITA_LINK_LIBS
  ${VITA_LIBDIR}/libSDL3_ttf.a
  ${VITA_LIBDIR}/libharfbuzz.a
  ${VITA_LIBDIR}/libSDL3_image.a
  ${VITA_LIBDIR}/libtiff.a
  ${VITA_LIBDIR}/libwebpmux.a
  ${VITA_LIBDIR}/libwebpdemux.a
  ${VITA_LIBDIR}/libwebp.a
  ${VITA_LIBDIR}/libsharpyuv.a
  ${VITA_LIBDIR}/libjpeg.a
  ${VITA_LIBDIR}/libfreetype.a
  ${VITA_LIBDIR}/libpng.a
  ${VITA_LIBDIR}/libbz2.a
  ${VITA_LIBDIR}/libz.a
  ${VITA_LIBDIR}/libvorbisfile.a
  ${VITA_LIBDIR}/libvorbis.a
  ${VITA_LIBDIR}/libogg.a
  ${VITA_LIBDIR}/libopenal.a
  ${VITA_LIBDIR}/libphysfs.a
  ${VITA_LIBDIR}/libfmt.a
  ${VITA_LIBDIR}/libSDL3.a
  SceGxm_stub
  SceDisplay_stub
  SceCtrl_stub
  SceAppMgr_stub
  SceAppUtil_stub
  SceAudio_stub
  SceAudioIn_stub
  SceSysmodule_stub
  SceIofilemgr_stub
  SceCommonDialog_stub
  SceTouch_stub
  SceHid_stub
  SceMotion_stub
  ScePower_stub
  SceProcessmgr_stub
  SceKernelDmacMgr_stub
  SceCamera_stub
  pthread
  m)

function(vita_disable_pic dir)
  get_property(targets DIRECTORY ${dir} PROPERTY BUILDSYSTEM_TARGETS)
  foreach(t ${targets})
    get_target_property(type ${t} TYPE)
    if(NOT type STREQUAL "INTERFACE_LIBRARY")
      set_target_properties(${t} PROPERTIES POSITION_INDEPENDENT_CODE OFF)
    endif()
  endforeach()
  get_property(subdirs DIRECTORY ${dir} PROPERTY SUBDIRECTORIES)
  foreach(d ${subdirs})
    vita_disable_pic(${d})
  endforeach()
endfunction()

function(supertux_vita_package target)
  include("${VITASDK}/share/vita.cmake" REQUIRED)

  target_link_libraries(${target} PRIVATE ${VITA_LINK_LIBS})
  vita_disable_pic(${PROJECT_SOURCE_DIR})

  vita_create_self(eboot.bin ${target})
  vita_create_vpk(supertux2.vpk ${VITA_TITLEID} eboot.bin
    VERSION ${VITA_VERSION}
    NAME ${VITA_APP_NAME}
    FILE ${PROJECT_SOURCE_DIR}/mk/vita/sce_sys/icon0.png sce_sys/icon0.png
    FILE ${PROJECT_SOURCE_DIR}/mk/vita/sce_sys/livearea/contents/bg.png sce_sys/livearea/contents/bg.png
    FILE ${PROJECT_SOURCE_DIR}/mk/vita/sce_sys/livearea/contents/startup.png sce_sys/livearea/contents/startup.png
    FILE ${PROJECT_SOURCE_DIR}/mk/vita/sce_sys/livearea/contents/template.xml sce_sys/livearea/contents/template.xml
  )
endfunction()
