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

project(jing_project VERSION 1.0.0 LANGUAGES C CXX)

# set main target
add_library(jing SHARED)
file(GLOB C_SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/*.c")
target_sources(jing PRIVATE ${C_SOURCES})

if(JING_ENABLE_TESTS)
    target_compile_definitions(jing PRIVATE JING_ENABLE_TESTS)
endif()

# add support for fetch series
include(FetchContent)

# build c-ares as a static library
FetchContent_Declare(
    cares
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
FetchContent_MakeAvailable(cares)
message(STATUS "Configuring c-ares succeed")
if(CMAKE_COMPILER_IS_GNUCC)
    target_compile_options(c-ares PRIVATE $<$<COMPILE_LANGUAGE:C,CXX>:-Wno-stringop-overflow -Wno-unused-result>)
endif()

# build wepoll
if(WIN32)
    FetchContent_Declare(
        wepoll
        GIT_REPOSITORY https://github.com/piscisaureus/wepoll.git
        GIT_TAG tags/v1.5.8
        GIT_SHALLOW TRUE
        SOURCE_SUBDIR none_exist
    )
    message(STATUS "Configuring wepoll")
    FetchContent_MakeAvailable(wepoll)
    message(STATUS "Configuring wepoll succeed")
    add_library(wepollstatic STATIC ${wepoll_SOURCE_DIR}/wepoll.c)
    target_include_directories(wepollstatic PUBLIC ${wepoll_SOURCE_DIR})
endif()

# build mimalloc as a static library
FetchContent_Declare(
    mimalloc
    GIT_REPOSITORY https://github.com/microsoft/mimalloc.git
    GIT_TAG tags/v3.1.5 
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
FetchContent_MakeAvailable(mimalloc)
message(STATUS "Configuring mimalloc succeed")

# build rpmalloc as a object library
FetchContent_Declare(
    rpmalloc
    GIT_REPOSITORY https://github.com/mjansson/rpmalloc.git
    GIT_TAG tags/1.4.5  
    GIT_SHALLOW TRUE
    SOURCE_SUBDIR none_exist
)
message(STATUS "Configuring rpmalloc")
FetchContent_MakeAvailable(rpmalloc)
message(STATUS "Configuring rpmalloc succeed")
add_library(rpmallocstatic STATIC ${rpmalloc_SOURCE_DIR}/rpmalloc/rpmalloc.c)
target_include_directories(rpmallocstatic PUBLIC ${rpmalloc_SOURCE_DIR}/rpmalloc)

# # build fastfloat as a header-only library
# FetchContent_Declare(
#   fast_float
#   GIT_REPOSITORY https://github.com/fastfloat/fast_float.git
#   GIT_TAG tags/v8.0.2
#   GIT_SHALLOW TRUE
# )
# message(STATUS "Configuring fast_float")
# FetchContent_MakeAvailable(fast_float)
# message(STATUS "Configuring fast_float succeed")
# target_include_directories(jing PRIVATE ${fast_float_SOURCE_DIR}/include)
# target_sources(jing PRIVATE src/jing_fastfloat.cpp)

# build zlib
FetchContent_Declare(
  zlibproject
  GIT_REPOSITORY https://github.com/madler/zlib.git
  GIT_TAG tags/v1.3.1
  GIT_SHALLOW TRUE
)
set(ZLIB_BUILD_EXAMPLES OFF CACHE BOOL "Whether to build zlib examples" FORCE)
message(STATUS "Configuring zlib")
FetchContent_MakeAvailable(zlibproject)
message(STATUS "Configuring zlib succeed")
if(WIN32)
    target_compile_options(zlibstatic PRIVATE $<$<COMPILE_LANGUAGE:C,CXX>:/wd4127 /wd4244>)
    target_compile_options(zlib PRIVATE $<$<COMPILE_LANGUAGE:C,CXX>:/wd4127 /wd4244>)
endif()

# build libdeflate
FetchContent_Declare(
  libdeflate
  GIT_REPOSITORY https://github.com/ebiggers/libdeflate.git
  GIT_TAG tags/v1.24
  GIT_SHALLOW TRUE
  SOURCE_SUBDIR none_exist
)
message(STATUS "Configuring libdeflate")
FetchContent_MakeAvailable(libdeflate)
message(STATUS "Configuring libdeflate succeed")
file(GLOB LIBDEFLATE_SOURCES CONFIGURE_DEPENDS
  "${libdeflate_SOURCE_DIR}/lib/*.c"
  "${libdeflate_SOURCE_DIR}/lib/*/*.c"
)
add_library(libdeflatestatic STATIC ${LIBDEFLATE_SOURCES})
target_include_directories(libdeflatestatic PUBLIC ${libdeflate_SOURCE_DIR})
if(WIN32)
    target_compile_options(libdeflatestatic PRIVATE $<$<COMPILE_LANGUAGE:C,CXX>:/wd4113>)
endif()

# build brotli
FetchContent_Declare(
    brotli
    GIT_REPOSITORY https://github.com/google/brotli.git
    GIT_TAG tags/v1.1.0
    GIT_SHALLOW TRUE
)
set(BUILD_SHARED_LIBS OFF CACHE BOOL "Whether to build brotli shared library" FORCE)
set(BUILD_TESTING     OFF CACHE BOOL "Whether to build brotli tests"          FORCE)
message(STATUS "Configuring brotli")
FetchContent_MakeAvailable(brotli)
message(STATUS "Configuring brotli succeed")
foreach(lib brotlicommon brotlienc brotlidec)
    if(WIN32)
        target_compile_options(${lib} PRIVATE $<$<COMPILE_LANGUAGE:C,CXX>:/wd4100 /wd4127 /wd4189 /wd4334>)
    endif()
endforeach()

# build zstd
FetchContent_Declare(
    zstd
    GIT_REPOSITORY https://github.com/facebook/zstd.git
    GIT_TAG tags/v1.5.7
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
FetchContent_MakeAvailable(zstd)
message(STATUS "Configuring zstd succeed")
if(CMAKE_COMPILER_IS_GNUCC)
    target_compile_options(libzstd_static PRIVATE $<$<COMPILE_LANGUAGE:C,CXX>:-Wno-maybe-uninitialized>)
endif()

# build boringssl
FetchContent_Declare(
    boringssl
    GIT_REPOSITORY https://github.com/google/boringssl.git
    GIT_TAG tags/0.20250818.0
    GIT_SHALLOW TRUE
)
if(WIN32)
    set(OPENSSL_NO_ASM ON CACHE BOOL "Whether to enable asm" FORCE)
endif()
message(STATUS "Configuring boringssl")
FetchContent_MakeAvailable(boringssl)
message(STATUS "Configuring boringssl succeed")

# build lsquic
FetchContent_Declare(
    lsquic
    GIT_REPOSITORY https://github.com/litespeedtech/lsquic.git
    GIT_TAG tags/v4.3.1
    GIT_SHALLOW TRUE
    SOURCE_SUBDIR none_exist
)
message(STATUS "Configuring lsquic")
FetchContent_MakeAvailable(lsquic)
message(STATUS "Configuring lsquic succeed")
set(LSQUIC_LIB_PATH ${lsquic_BINARY_DIR}/${CMAKE_STATIC_LIBRARY_PREFIX}lsquic${CMAKE_STATIC_LIBRARY_SUFFIX})
set(LSQUIC_CONFIGURE_ARGS
    "cmake --preset=${PRESET_NAME}"
    "-DLIBSSL_DIR=\"${boringssl_SOURCE_DIR}\""
    "-DLIBSSL_LIB=\"${boringssl_BINARY_DIR}\""
    "-DCMAKE_INCLUDE_PATH=\"${zlib_SOURCE_DIR}\""
    "-DCMAKE_LIBRARY_PATH=\"${zlib_BINARY_DIR}\""
    "-DCMAKE_ARCHIVE_OUTPUT_DIRECTORY=\"${lsquic_BINARY_DIR}\""
    "-DLSQUIC_BIN=OFF"
    "-DLSQUIC_TESTS=OFF"
)
list(JOIN LSQUIC_CONFIGURE_ARGS " " LSQUIC_CONFIGURE_COMMAND)
if(WIN32)
    file(WRITE ${lsquic_SOURCE_DIR}/build_lsquic.ps1 
"Copy-Item -Path \"${CMAKE_SOURCE_DIR}/CMakePresets.json\" -Destination \"${lsquic_SOURCE_DIR}\" 
Copy-Item -Path \"${zlib_BINARY_DIR}/zconf.h\" -Destination \"${zlib_SOURCE_DIR}\"
${LSQUIC_CONFIGURE_COMMAND}
cmake --build build"
    )
    add_custom_command(
        OUTPUT  ${LSQUIC_LIB_PATH}
        COMMAND pwsh -File build_lsquic.ps1
        WORKING_DIRECTORY ${lsquic_SOURCE_DIR}
        DEPENDS ssl crypto zlibstatic
        VERBATIM
    )
else()
    file(WRITE ${lsquic_SOURCE_DIR}/build_lsquic.sh 
"#!/bin/sh
set -e
cp \"${CMAKE_SOURCE_DIR}/CMakePresets.json\" \"${lsquic_SOURCE_DIR}\"
cp \"${zlib_BINARY_DIR}/zconf.h\" \"${zlib_SOURCE_DIR}\"
${LSQUIC_CONFIGURE_COMMAND}
cmake --build build"
    )
    add_custom_command(
        OUTPUT  ${LSQUIC_LIB_PATH}
        COMMAND chmod +x build_lsquic.sh 
        COMMAND sh build_lsquic.sh
        WORKING_DIRECTORY ${lsquic_SOURCE_DIR}
        DEPENDS ssl crypto zlibstatic
        VERBATIM
    )
endif()
add_custom_target(lsquicbuild DEPENDS ${LSQUIC_LIB_PATH})
add_library(lsquicstatic INTERFACE)
add_dependencies(lsquicstatic lsquicbuild)
target_link_libraries(lsquicstatic INTERFACE ${LSQUIC_LIB_PATH} ssl crypto zlibstatic)
target_include_directories(lsquicstatic INTERFACE ${lsquic_SOURCE_DIR}/include)

target_link_libraries(jing PRIVATE c-ares lsquicstatic ssl crypto zlibstatic libdeflatestatic libzstd_static brotlienc brotlidec brotlicommon mimalloc-static rpmallocstatic)
if(WIN32)
    target_link_libraries(jing PRIVATE wepollstatic)
endif()