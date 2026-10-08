set(CMAKE_EXECUTABLE_SUFFIX .html)
set(IS_EMSCRIPTEN_BUILD ON)
if(NOT EMSCRIPTEN_VERSION VERSION_EQUAL "6.0.11")
  message(FATAL_ERROR "Browser artifact identity requires the pinned Emscripten 6.0.11 toolchain")
endif()
set(SQ_DISABLE_INSTALLER YES)
set(SSQ_BUILD_INSTALL NO)

set(EM_USE_FLAGS "-sDISABLE_EXCEPTION_CATCHING=0 -fPIC")
option(WEB_FULL_PRELOAD "Preload the complete soundtrack (rollback/comparison)" OFF)
option(WEB_COMPRESS_ASSETS "Publish gzip startup packages with raw fallback" ON)
find_package(Python3 REQUIRED COMPONENTS Interpreter)
execute_process(COMMAND git rev-parse HEAD WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}"
  OUTPUT_VARIABLE WEB_SOURCE_COMMIT OUTPUT_STRIP_TRAILING_WHITESPACE COMMAND_ERROR_IS_FATAL ANY)
execute_process(COMMAND git rev-parse --git-path HEAD WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}"
  OUTPUT_VARIABLE WEB_GIT_HEAD OUTPUT_STRIP_TRAILING_WHITESPACE COMMAND_ERROR_IS_FATAL ANY)
execute_process(COMMAND git symbolic-ref -q HEAD WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}"
  OUTPUT_VARIABLE WEB_GIT_REF OUTPUT_STRIP_TRAILING_WHITESPACE)
set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS "${PROJECT_SOURCE_DIR}/${WEB_GIT_HEAD}")
if(WEB_GIT_REF)
  set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS "${PROJECT_SOURCE_DIR}/.git/${WEB_GIT_REF}")
endif()
set(WEB_PACKAGER "${PROJECT_SOURCE_DIR}/tools/web/package_assets.py")
set(WEB_ASSETS_DIR "${CMAKE_BINARY_DIR}/web-assets")
set(WEB_PACKAGING_ARGS prepare --source "${PROJECT_SOURCE_DIR}/data" --output "${WEB_ASSETS_DIR}"
  --source-commit "${WEB_SOURCE_COMMIT}" --configuration "${CMAKE_BUILD_TYPE}" --runtime-root "${BUILD_CONFIG_DATA_DIR}" --presentation)
if(WEB_FULL_PRELOAD)
  list(APPEND WEB_PACKAGING_ARGS --full-preload)
endif()
# Prepare before the first link; subsequent builds also depend on every input.
execute_process(COMMAND "${Python3_EXECUTABLE}" "${WEB_PACKAGER}" ${WEB_PACKAGING_ARGS}
  COMMAND_ERROR_IS_FATAL ANY)
# SDL3_image decodes real file bytes; browser preload plugins are unnecessary.
set(EM_LINK_FLAGS " -sINITIAL_MEMORY=134217728 -sALLOW_MEMORY_GROWTH=1 -sMAXIMUM_MEMORY=536870912 -sERROR_ON_UNDEFINED_SYMBOLS=1 --preload-file ${WEB_ASSETS_DIR}/startup@${BUILD_CONFIG_DATA_DIR} --pre-js ${WEB_ASSETS_DIR}/identity.js --pre-js ${PROJECT_SOURCE_DIR}/mk/emscripten/storage.js --pre-js ${PROJECT_SOURCE_DIR}/mk/emscripten/browser.js -lidbfs.js")
# SDK 6.0.11 omits wasmBinary from its default incoming API. Preserve the
# default APIs and explicitly accept the already validated/cached WASM bytes.
string(APPEND EM_LINK_FLAGS " -sINCOMING_MODULE_JS_API=ENVIRONMENT,arguments,canvas,dynamicLibraries,elementPointerLock,instantiateWasm,locateFile,monitorRunDependencies,noExitRuntime,noInitialRun,onAbort,onExit,onRuntimeInitialized,postRun,preInit,preRun,print,printErr,setStatus,statusMessage,stderr,stdin,stdout,thisProgram,wasm,websocket,wasmBinary")
if(ENABLE_OPENGL)
  set(EM_LINK_FLAGS "${EM_LINK_FLAGS} -sFULL_ES2")
  set(HAVE_OPENGL ON CACHE BOOL "")
  set(USE_OPENGLES2 ON CACHE BOOL "")
endif()

if(CMAKE_BUILD_TYPE MATCHES Debug)
  set(EM_USE_FLAGS "${EM_USE_FLAGS} -fsanitize=undefined")
  set(EM_LINK_FLAGS "${EM_LINK_FLAGS} -fsanitize=undefined -sSAFE_HEAP=1 -sASSERTIONS=1")
endif()
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${EM_USE_FLAGS} ${EM_C_FLAGS}")
if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
  string(APPEND CMAKE_CXX_FLAGS " -Wno-lifetime-safety-intra-tu-suggestions")
endif()
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${EM_USE_FLAGS} ${EM_C_FLAGS}")
# Do not append these twice: a repeated --pre-js evaluates the storage initializer
# twice and tries to mount/hydrate the same directory concurrently.
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${EM_LINK_FLAGS}")

add_library(OpenAL INTERFACE IMPORTED)
set_target_properties(OpenAL PROPERTIES
  INTERFACE_INCLUDE_DIRECTORIES "${PROJECT_SOURCE_DIR}/mk/emscripten/AL"
  INTERFACE_LINK_LIBRARIES "-lopenal"
)

function(supertux_finalize_web_assets)
  file(GLOB_RECURSE WEB_ASSET_INPUTS CONFIGURE_DEPENDS "${PROJECT_SOURCE_DIR}/data/*")
  add_custom_command(OUTPUT "${WEB_ASSETS_DIR}/inventory.json" "${WEB_ASSETS_DIR}/identity.js"
    COMMAND "${Python3_EXECUTABLE}" "${WEB_PACKAGER}" ${WEB_PACKAGING_ARGS}
    DEPENDS ${WEB_ASSET_INPUTS} "${WEB_PACKAGER}" "${PROJECT_SOURCE_DIR}/tools/web/coop_presentation.py" "${PROJECT_SOURCE_DIR}/tools/web/coop/scene.json" VERBATIM)
  add_custom_target(web_assets DEPENDS "${WEB_ASSETS_DIR}/inventory.json" "${WEB_ASSETS_DIR}/identity.js")
  add_dependencies(supertux2 web_assets)
  set_property(TARGET supertux2 APPEND PROPERTY LINK_DEPENDS "${WEB_ASSETS_DIR}/inventory.json" "${WEB_ASSETS_DIR}/identity.js"
    "${PROJECT_SOURCE_DIR}/mk/emscripten/assets.js" "${PROJECT_SOURCE_DIR}/mk/emscripten/template.html.in"
    "${PROJECT_SOURCE_DIR}/mk/emscripten/coop.js" "${PROJECT_SOURCE_DIR}/mk/emscripten/coop-controller.html"
    "${PROJECT_SOURCE_DIR}/mk/emscripten/coop-view.js" "${PROJECT_SOURCE_DIR}/mk/emscripten/coop-view.html")
  set(WEB_ASSEMBLY_ARGS assemble --build "${CMAKE_BINARY_DIR}" --output "${CMAKE_BINARY_DIR}")
  if(NOT WEB_COMPRESS_ASSETS)
    list(APPEND WEB_ASSEMBLY_ARGS --no-compression)
  endif()
  add_custom_command(TARGET supertux2 POST_BUILD
    COMMAND "${Python3_EXECUTABLE}" "${WEB_PACKAGER}" ${WEB_ASSEMBLY_ARGS} VERBATIM)
endfunction()
cmake_language(DEFER CALL supertux_finalize_web_assets)
