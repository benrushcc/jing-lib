cmake_minimum_required(VERSION 3.24)

# build using this command : cmake --preset=windows or macos or linux && cmake --build build

option(JING_ENABLE_TESTS "Enable test functions in the library" OFF)
option(JING_ENABLE_LTO   "Enable Link Time Optimization"        ON)

# set proxy if possible
if(DEFINED HTTP_PROXY)
    set(ENV{HTTP_PROXY} "${HTTP_PROXY}")
endif()
if(DEFINED HTTPS_PROXY)
    set(ENV{HTTPS_PROXY} "${HTTPS_PROXY}")
endif()

# detecting presets
set(PRESET_NAME $ENV{JING_PRESET_NAME})
if(DEFINED PRESET_NAME)
    message(STATUS "Preset name : ${PRESET_NAME}")
else()
    message(FATAL_ERROR "Preset not found")
endif()

# don't build thirdparty shared libraries
set(BUILD_SHARED_LIBS OFF CACHE BOOL "cmake shared library policy" FORCE)

# set apple env
if(APPLE)
    execute_process(
        COMMAND xcrun --sdk macosx --show-sdk-path
        RESULT_VARIABLE XCRUN_RESULT
        OUTPUT_VARIABLE MACOS_SDK_PATH
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )
    if(NOT XCRUN_RESULT EQUAL 0)
        message(FATAL_ERROR "Failed to get macOS SDK path")
    endif()
    set(CMAKE_OSX_SYSROOT       ${MACOS_SDK_PATH}   CACHE STRING "macos sdk root"     FORCE)
    set(CMAKE_OSX_ARCHITECTURES "arm64"             CACHE STRING "macos architecture" FORCE)
endif()

project(jing_project VERSION 1.0.0 LANGUAGES C)

# set main target
add_library(jing SHARED)
target_sources(jing PRIVATE 
    src/jing_common.c
    src/jing_mimalloc.c
    src/jing_rpmalloc.c
    src/jing_deflate.c
    src/jing_brotli.c
    src/jing_zstd.c
    src/jing_c_ares.c
    src/jing_ssl.c
    src/jing_quic.c
)

if(JING_ENABLE_TESTS)
    target_compile_definitions(jing PRIVATE JING_ENABLE_TESTS)
endif()

# set library versions
set(JING_TOMLC17_VERSION "R250712" CACHE STRING "tomlc17 library version" FORCE)
set(JING_YYJSON_VERSION "0.12.0" CACHE STRING "yyjson library version" FORCE)
set(JING_WEPOLL_VERSION "v1.5.8" CACHE STRING "wepoll library version" FORCE)
set(JING_MIMALLOC_VERSION "v3.1.5" CACHE STRING "mimalloc library version" FORCE)
set(JING_RPMALLOC_VERSION "1.4.5" CACHE STRING "rpmalloc library version" FORCE)
set(JING_ZLIB_VERSION "v1.3.1" CACHE STRING "zlib library version" FORCE)
set(JING_LIBDEFLATE_VERSION "v1.24" CACHE STRING "libdeflate library version" FORCE)
set(JING_BROTLI_VERSION "v1.1.0" CACHE STRING "brotli library version" FORCE)
set(JING_ZSTD_VERSION "v1.5.7" CACHE STRING "zstd library version" FORCE)
set(JING_CARES_VERSION "v1.34.5" CACHE STRING "c-ares library version" FORCE)
set(JING_BORINGSSL_VERSION "0.20250818.0" CACHE STRING "boringssl library version" FORCE)
set(JING_LSQUIC_VERSION "v4.3.1" CACHE STRING "lsquic library version" FORCE)

# add support for fetch series
include(FetchContent)

# build tomlc17
FetchContent_Declare(
    tomlc17project
    GIT_REPOSITORY https://github.com/cktan/tomlc17.git
    GIT_TAG tags/${JING_TOMLC17_VERSION}
    GIT_SHALLOW TRUE
    SOURCE_SUBDIR none_exist
)
message(STATUS "Configuring tomlc17")
FetchContent_MakeAvailable(tomlc17project)
message(STATUS "Configuring tomlc17 succeed")
add_library(tomlc17static STATIC ${tomlc17project_SOURCE_DIR}/src/tomlc17.c)
target_include_directories(tomlc17static PUBLIC ${tomlc17project_SOURCE_DIR}/src)

# build yyjson
FetchContent_Declare(
    yyjsonproject
    GIT_REPOSITORY https://github.com/ibireme/yyjson.git
    GIT_TAG tags/${JING_YYJSON_VERSION}
    GIT_SHALLOW TRUE
)
set(YYJSON_INSTALL OFF CACHE BOOL "Whether to generate yyjson installation target" FORCE)
message(STATUS "Configuring yyjson")
FetchContent_MakeAvailable(yyjsonproject)
message(STATUS "Configuring yyjson succeed")

# build wepoll
if(WIN32)
    FetchContent_Declare(
        wepollproject
        GIT_REPOSITORY https://github.com/piscisaureus/wepoll.git
        GIT_TAG tags/${JING_WEPOLL_VERSION}
        GIT_SHALLOW TRUE
        SOURCE_SUBDIR none_exist
    )
    message(STATUS "Configuring wepoll")
    FetchContent_MakeAvailable(wepollproject)
    message(STATUS "Configuring wepoll succeed")
    add_library(wepollstatic STATIC ${wepollproject_SOURCE_DIR}/wepoll.c)
    target_include_directories(wepollstatic PUBLIC ${wepollproject_SOURCE_DIR})
    target_link_libraries(wepollstatic PUBLIC ws2_32)
endif()

# build mimalloc as a static library
FetchContent_Declare(
    mimallocproject
    GIT_REPOSITORY https://github.com/microsoft/mimalloc.git
    GIT_TAG tags/${JING_MIMALLOC_VERSION} 
    GIT_SHALLOW TRUE
)
set(MI_BUILD_SHARED OFF CACHE BOOL "Whether to build mimalloc shared library" FORCE)
set(MI_BUILD_STATIC ON  CACHE BOOL "Whether to build mimalloc static library" FORCE)
set(MI_BUILD_OBJECT OFF CACHE BOOL "Whether to build mimalloc object library" FORCE)
set(MI_BUILD_TESTS  OFF CACHE BOOL "Whether to build mimalloc tests"          FORCE)
set(MI_OVERRIDE     OFF CACHE BOOL "Whether to override malloc and free"      FORCE)
set(MI_OPT_ARCH     ON  CACHE BOOL "Whether to enable arch optimizations"     FORCE)
set(MI_OPT_SIMD     ON  CACHE BOOL "Whether to enable SIMD optimizations"     FORCE)
message(STATUS "Configuring mimalloc")
FetchContent_MakeAvailable(mimallocproject)
message(STATUS "Configuring mimalloc succeed")

# build rpmalloc as a object library
FetchContent_Declare(
    rpmallocproject
    GIT_REPOSITORY https://github.com/mjansson/rpmalloc.git
    GIT_TAG tags/${JING_RPMALLOC_VERSION}  
    GIT_SHALLOW TRUE
    SOURCE_SUBDIR none_exist
)
message(STATUS "Configuring rpmalloc")
FetchContent_MakeAvailable(rpmallocproject)
message(STATUS "Configuring rpmalloc succeed")
add_library(rpmallocstatic STATIC ${rpmallocproject_SOURCE_DIR}/rpmalloc/rpmalloc.c)
target_include_directories(rpmallocstatic PUBLIC ${rpmallocproject_SOURCE_DIR}/rpmalloc)

# build zlib
FetchContent_Declare(
  zlibproject
  GIT_REPOSITORY https://github.com/madler/zlib.git
  GIT_TAG tags/${JING_ZLIB_VERSION}
  GIT_SHALLOW TRUE
)
set(ZLIB_BUILD_EXAMPLES OFF CACHE BOOL "Whether to build zlib examples" FORCE)
message(STATUS "Configuring zlib")
FetchContent_MakeAvailable(zlibproject)
message(STATUS "Configuring zlib succeed")
set_target_properties(zlib PROPERTIES EXCLUDE_FROM_ALL TRUE)
if(WIN32)
    target_compile_options(zlibstatic PRIVATE $<$<COMPILE_LANGUAGE:C,CXX>:/wd4127 /wd4244>)
endif()

# build libdeflate
FetchContent_Declare(
  libdeflateproject
  GIT_REPOSITORY https://github.com/ebiggers/libdeflate.git
  GIT_TAG tags/${JING_LIBDEFLATE_VERSION}
  GIT_SHALLOW TRUE
)
set(LIBDEFLATE_BUILD_SHARED_LIB OFF CACHE BOOL "Whether to build libdeflate shared library" FORCE)
set(LIBDEFLATE_BUILD_GZIP       OFF CACHE BOOL "Whether to build libdeflate gzip program"   FORCE)
message(STATUS "Configuring libdeflate")
FetchContent_MakeAvailable(libdeflateproject)
message(STATUS "Configuring libdeflate succeed")
if(WIN32)
    target_compile_options(libdeflate_static PRIVATE $<$<COMPILE_LANGUAGE:C,CXX>:/wd4113>)
endif()

# build brotli
FetchContent_Declare(
    brotliproject
    GIT_REPOSITORY https://github.com/google/brotli.git
    GIT_TAG tags/${JING_BROTLI_VERSION}
    GIT_SHALLOW TRUE
)
set(BUILD_TESTING OFF CACHE BOOL "Whether to build brotli tests" FORCE)
message(STATUS "Configuring brotli")
FetchContent_MakeAvailable(brotliproject)
message(STATUS "Configuring brotli succeed")
foreach(lib brotlicommon brotlienc brotlidec)
    if(WIN32)
        target_compile_options(${lib} PRIVATE $<$<COMPILE_LANGUAGE:C,CXX>:/wd4100 /wd4127 /wd4189 /wd4334>)
    endif()
endforeach()

# build zstd
FetchContent_Declare(
    zstdproject
    GIT_REPOSITORY https://github.com/facebook/zstd.git
    GIT_TAG tags/${JING_ZSTD_VERSION}
    GIT_SHALLOW TRUE
    SOURCE_SUBDIR build/cmake
)
set(ZSTD_BUILD_SHARED   OFF CACHE BOOL "Whether to build zstd shared library" FORCE)
set(ZSTD_BUILD_STATIC   ON  CACHE BOOL "Whether to build zstd static library" FORCE)
set(ZSTD_BUILD_PROGRAMS OFF CACHE BOOL "Whether to build zstd program"        FORCE)
set(ZSTD_BUILD_TESTS    OFF CACHE BOOL "Whether to build zstd tests"          FORCE)
set(ZSTD_BUILD_CONTRIB  OFF CACHE BOOL "Whether to build zstd contrib"        FORCE)
set(ZSTD_LEGACY_SUPPORT OFF CACHE BOOL "Whether to enable legacy format"      FORCE)
message(STATUS "Configuring zstd")
FetchContent_MakeAvailable(zstdproject)
message(STATUS "Configuring zstd succeed")
if(CMAKE_COMPILER_IS_GNUCC)
    target_compile_options(libzstd_static PRIVATE $<$<COMPILE_LANGUAGE:C,CXX>:-Wno-maybe-uninitialized>)
endif()

# build c-ares as a static library
FetchContent_Declare(
    caresproject
    GIT_REPOSITORY https://github.com/c-ares/c-ares.git
    GIT_TAG tags/v1.34.5
    GIT_SHALLOW TRUE
)
set(CARES_STATIC        ON  CACHE BOOL "Whether to build c-ares static library"   FORCE)
set(CARES_SHARED        OFF CACHE BOOL "Whether to build c-ares shared library"   FORCE)
set(CARES_INSTALL       OFF CACHE BOOL "Whether to hook c-ares in installation"   FORCE)
set(CARES_SYMBOL_HIDING ON  CACHE BOOL "Whether to hide c-ares private symbols"   FORCE)
set(CARES_STATIC_PIC    ON  CACHE BOOL "Whether to build as position-independent" FORCE)
set(CARES_BUILD_TOOLS   OFF CACHE BOOL "Whether to build c-ares tools"            FORCE)
message(STATUS "Configuring c-ares")
FetchContent_MakeAvailable(caresproject)
message(STATUS "Configuring c-ares succeed")
if(CMAKE_COMPILER_IS_GNUCC)
    target_compile_options(c-ares PRIVATE $<$<COMPILE_LANGUAGE:C,CXX>:-Wno-stringop-overflow -Wno-unused-result>)
endif()

# build boringssl
FetchContent_Declare(
    boringsslproject
    GIT_REPOSITORY https://github.com/google/boringssl.git
    GIT_TAG tags/${JING_BORINGSSL_VERSION}
    GIT_SHALLOW TRUE
)
if(WIN32)
    set(OPENSSL_NO_ASM ON CACHE BOOL "Whether to enable asm" FORCE)
endif()
message(STATUS "Configuring boringssl")
FetchContent_MakeAvailable(boringsslproject)
message(STATUS "Configuring boringssl succeed")
set_target_properties(bssl PROPERTIES EXCLUDE_FROM_ALL TRUE)
if(APPLE)
    target_compile_definitions(crypto PRIVATE OPENSSL_STATIC_ARMCAP)
    target_compile_definitions(ssl PRIVATE OPENSSL_STATIC_ARMCAP)
endif()

# build lsquic
FetchContent_Declare(
    lsquicproject
    GIT_REPOSITORY https://github.com/litespeedtech/lsquic.git
    GIT_TAG tags/${JING_LSQUIC_VERSION}
    GIT_SHALLOW TRUE
    SOURCE_SUBDIR none_exist
)
message(STATUS "Configuring lsquic")
FetchContent_MakeAvailable(lsquicproject)
message(STATUS "Configuring lsquic succeed")
set(LSQUIC_LIB_PATH ${lsquicproject_BINARY_DIR}/${CMAKE_STATIC_LIBRARY_PREFIX}lsquic${CMAKE_STATIC_LIBRARY_SUFFIX})
set(LSQUIC_CONFIGURE_ARGS
    "cmake --preset=${PRESET_NAME}"
    "-DLIBSSL_DIR=\"${boringsslproject_SOURCE_DIR}\""
    "-DCMAKE_INCLUDE_PATH=\"${zlibproject_SOURCE_DIR}\""
    "-DCMAKE_LIBRARY_PATH=\"${zlibproject_BINARY_DIR}\""
    "-DCMAKE_ARCHIVE_OUTPUT_DIRECTORY=\"${lsquicproject_BINARY_DIR}\""
    "-DLSQUIC_BIN=OFF"
    "-DLSQUIC_TESTS=OFF"
)
list(JOIN LSQUIC_CONFIGURE_ARGS " " LSQUIC_CONFIGURE_COMMAND)
if(WIN32)
    file(WRITE ${lsquicproject_SOURCE_DIR}/build_lsquic.ps1 
"Copy-Item -Path \"${boringsslproject_BINARY_DIR}/crypto.lib\" -Destination \"${boringsslproject_SOURCE_DIR}\"
Copy-Item -Path \"${boringsslproject_BINARY_DIR}/ssl.lib\" -Destination \"${lsquicproject_SOURCE_DIR}\"
Copy-Item -Path \"${CMAKE_SOURCE_DIR}/CMakePresets.json\" -Destination \"${lsquicproject_SOURCE_DIR}\" 
Copy-Item -Path \"${zlibproject_BINARY_DIR}/zconf.h\" -Destination \"${zlibproject_SOURCE_DIR}\"
${LSQUIC_CONFIGURE_COMMAND}
cmake --build build"
    )
    add_custom_command(
        OUTPUT  ${LSQUIC_LIB_PATH}
        COMMAND pwsh -File build_lsquic.ps1
        WORKING_DIRECTORY ${lsquicproject_SOURCE_DIR}
        DEPENDS ssl crypto zlibstatic
        VERBATIM
    )
else()
    file(WRITE ${lsquicproject_SOURCE_DIR}/build_lsquic.sh 
"#!/bin/sh
set -e
cp \"${boringsslproject_BINARY_DIR}/libcrypto.a\" \"${boringsslproject_SOURCE_DIR}\"
cp \"${boringsslproject_BINARY_DIR}/libssl.a\" \"${boringsslproject_SOURCE_DIR}\"
cp \"${CMAKE_SOURCE_DIR}/CMakePresets.json\" \"${lsquicproject_SOURCE_DIR}\"
cp \"${zlibproject_BINARY_DIR}/zconf.h\" \"${zlibproject_SOURCE_DIR}\"
${LSQUIC_CONFIGURE_COMMAND}
cmake --build build"
    )
    add_custom_command(
        OUTPUT  ${LSQUIC_LIB_PATH}
        COMMAND chmod +x build_lsquic.sh 
        COMMAND sh build_lsquic.sh
        WORKING_DIRECTORY ${lsquicproject_SOURCE_DIR}
        DEPENDS ssl crypto zlibstatic
        VERBATIM
    )
endif()
add_custom_target(lsquicbuild DEPENDS ${LSQUIC_LIB_PATH})
add_library(lsquicstatic INTERFACE)
add_dependencies(lsquicstatic lsquicbuild)
target_link_libraries(lsquicstatic INTERFACE ${LSQUIC_LIB_PATH} ssl crypto zlibstatic)
target_include_directories(lsquicstatic INTERFACE ${lsquicproject_SOURCE_DIR}/include)

target_link_libraries(jing PRIVATE tomlc17static yyjson c-ares lsquicstatic ssl crypto zlibstatic libdeflate_static libzstd_static brotlienc brotlidec brotlicommon mimalloc-static rpmallocstatic)
if(WIN32)
    target_link_libraries(jing PRIVATE wepollstatic)
endif()