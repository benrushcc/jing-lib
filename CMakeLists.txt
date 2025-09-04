cmake_minimum_required(VERSION 3.24)

# treat warning as errors
set(CMAKE_COMPILER_WARNING_AS_ERRORS ON)

# # set c standard
# set(CMAKE_C_STANDARD 11)
# set(CMAKE_C_STANDARD_REQUIRED ON)

# set proxy
if(DEFINED HTTP_PROXY)
    set(ENV{HTTP_PROXY} "${HTTP_PROXY}")
endif()
if(DEFINED HTTPS_PROXY)
    set(ENV{HTTPS_PROXY} "${HTTPS_PROXY}")
endif()

# check macOS architecture
if(APPLE)
    execute_process(
        COMMAND uname -m
        OUTPUT_VARIABLE MACOS_ARCH
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )
    if(NOT MACOS_ARCH STREQUAL "arm64")
        message(FATAL_ERROR "Jing project only supports Apple Silicon (ARM64) Macs. Current architecture: ${CURRENT_ARCH}")
    endif()
endif()

# Enable LTO (Link Time Optimization) for all platforms
include(CheckIPOSupported)
check_ipo_supported(RESULT result)
if(result)
    set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)
endif()


# set compiler toolchain
if(WIN32)
    set(CMAKE_C_COMPILER cl)
    set(CMAKE_CXX_COMPILER cl)
elseif(APPLE)
    set(CMAKE_C_COMPILER clang)
    set(CMAKE_CXX_COMPILER clang++)
elseif(UNIX)
    set(CMAKE_C_COMPILER gcc)
    set(CMAKE_CXX_COMPILER g++)
endif()

project(jing_project VERSION 1.0.0 LANGUAGES C CXX)

# build using this command : cmake -GNinja -B build -DCMAKE_BUILD_TYPE=Release -DJING_ENABLE_TESTS=ON && cmake --build build --config Release --verbose

# whether to enable test functions for jing library
option(JING_ENABLE_TESTS "Enable test functions in the library" OFF)

# use release build by default
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Build type" FORCE)
endif()

# add support for fetch series
include(FetchContent)

# check for windows environment 
if(WIN32)
    execute_process(
        COMMAND nasm -v
        RESULT_VARIABLE NASM_RESULT
        OUTPUT_QUIET
        ERROR_QUIET
    )
    if(NASM_RESULT EQUAL 0)
        message(STATUS "Nasm found")
        enable_language(ASM_NASM)
    else()
        message(FATAL_ERROR "Nasm not found, please install it first")
    endif()
    execute_process(
        COMMAND perl -v
        RESULT_VARIABLE PERL_RESULT
        OUTPUT_QUIET
        ERROR_QUIET
    )
    if(PERL_RESULT EQUAL 0)
        message(STATUS "Perl found")
    else()
        message(FATAL_ERROR "Perl not found, please install it first")
    endif()
    # using vswhere to find vcvars64.bat path
    find_program(VSWHERE_PATH vswhere
        PATHS "$ENV{ProgramFiles\(x86\)}/Microsoft Visual Studio/Installer"
        "$ENV{ProgramFiles}/Microsoft Visual Studio/Installer"
    )
    if(VSWHERE_PATH)
        execute_process(
            COMMAND "${VSWHERE_PATH}" -latest -property installationPath
            OUTPUT_VARIABLE VS_INSTALL_PATH
            OUTPUT_STRIP_TRAILING_WHITESPACE
        )
        if(VS_INSTALL_PATH)
            set(VCVARS_PATH "${VS_INSTALL_PATH}/VC/Auxiliary/Build/vcvars64.bat" CACHE PATH "Path to vcvars64.bat")
            if(EXISTS "${VCVARS_PATH}")
                message(STATUS "Found vcvars64.bat: ${VCVARS_PATH}")
            else()
                message(FATAL_ERROR "vcvars64.bat not found at ${VCVARS_PATH}")
            endif()
        else()
            message(FATAL_ERROR "Could not determine Visual Studio installation path")
        endif()
    else()
        message(FATAL_ERROR "vswhere not found, please install visual studio first")
    endif()
endif()

# our target
add_library(jing SHARED 
    src/jing_common.c 
    src/jing_buffer.c 
    # src/jing_fastfloat.cpp 
    # src/jing_malloc.c 
    # src/jing_rpmalloc.c 
    # src/jing_mimalloc.c
    # src/jing_deflate.c
    # src/jing_brotli.c
    # src/jing_zstd.c
    # src/jing_c_ares.c
    src/jing_ssl.c
)

# compiler options for our target
if(WIN32)
    set(JING_COMPILE_OPTIONS "/MD;/O2;/W3;/GF;/nologo;/wd4819")
    set(JING_LINK_OPTIONS "/OPT:REF;/OPT:ICF;/DEBUG:NONE")
elseif(APPLE)
    set(JING_COMPILE_OPTIONS "-O3;-Wall;-march=native;-fvisibility=hidden")
    set(JING_LINK_OPTIONS "-dead_strip;-Wl,-x")
elseif(UNIX)
    set(JING_COMPILE_OPTIONS "-O3;-Wall;-march=native;-fvisibility=hidden")
    set(JING_LINK_OPTIONS "-Wl,--strip-all")
endif()
target_compile_options(jing PRIVATE ${JING_COMPILE_OPTIONS})
target_link_options(jing PRIVATE ${JING_LINK_OPTIONS})

if(JING_ENABLE_TESTS)
    target_compile_definitions(jing PRIVATE JING_ENABLE_TESTS)
endif()



# # build c-ares as a static library
# FetchContent_Declare(
#     cares
#     GIT_REPOSITORY https://github.com/c-ares/c-ares.git
#     GIT_TAG tags/v1.34.5
#     GIT_SHALLOW TRUE
# )
# set(CARES_STATIC ON CACHE BOOL "Whether to build c-ares static library" FORCE)
# set(CARES_SHARED OFF CACHE BOOL "Whether to build c-ares shared library" FORCE)
# set(CARES_INSTALL OFF CACHE BOOL "Whether to hook c-ares in installation" FORCE)
# set(CARES_SYMBOL_HIDING ON CACHE BOOL "Whether to hide c-ares private symbols" FORCE)
# set(CARES_STATIC_PIC ON CACHE BOOL "Whether to build c-ares static library as position-independent" FORCE)
# set(CARES_BUILD_TOOLS OFF CACHE BOOL "Whether to build c-ares tools" FORCE)
# message(STATUS "Configuring c-ares")
# FetchContent_MakeAvailable(cares)
# message(STATUS "Configuring c-ares succeed")
# target_compile_options(c-ares PRIVATE ${JING_COMPILE_OPTIONS})
# target_include_directories(jing PRIVATE ${cares_SOURCE_DIR}/include)
# target_link_libraries(jing PRIVATE c-ares)


# # build wepoll as a object library
# if(WIN32)
#     FetchContent_Declare(
#         wepoll
#         GIT_REPOSITORY https://github.com/piscisaureus/wepoll.git
#         GIT_TAG tags/v1.5.8
#         GIT_SHALLOW TRUE
#     )
#     message(STATUS "Configuring wepoll")
#     FetchContent_MakeAvailable(wepoll)
#     message(STATUS "Configuring wepoll succeed")
#     add_library(wepoll_lib OBJECT ${wepoll_SOURCE_DIR}/wepoll.c)
#     target_compile_options(wepoll_lib PRIVATE ${JING_COMPILE_OPTIONS})
#     target_include_directories(jing PRIVATE ${wepoll_SOURCE_DIR})
#     target_link_libraries(jing PRIVATE ws2_32 $<TARGET_OBJECTS:wepoll_lib>)
# endif()

# # build mimalloc as a static library
# FetchContent_Declare(
#     mimalloc
#     GIT_REPOSITORY https://github.com/microsoft/mimalloc.git
#     GIT_TAG tags/v3.1.5 
#     GIT_SHALLOW TRUE
# )
# set(MI_BUILD_SHARED OFF CACHE BOOL "Whether to build mimalloc shared library" FORCE)
# set(MI_BUILD_STATIC ON CACHE BOOL  "Whether to build mimalloc static library" FORCE)
# set(MI_BUILD_OBJECT OFF CACHE BOOL "Whether to build mimalloc object library" FORCE)
# set(MI_BUILD_TESTS OFF CACHE BOOL  "Whether to build mimalloc tests" FORCE)
# set(MI_OVERRIDE OFF CACHE BOOL "Whether to override malloc and free" FORCE)
# set(MI_OPT_ARCH ON CACHE BOOL "Whether to enable architecture specific optimizations" FORCE)
# set(MI_OPT_SIMD ON CACHE BOOL "Whether to enable SIMD optimizations" FORCE)
# message(STATUS "Configuring mimalloc")
# FetchContent_MakeAvailable(mimalloc)
# message(STATUS "Configuring mimalloc succeed")
# target_compile_options(mimalloc-static PRIVATE ${JING_COMPILE_OPTIONS})
# target_include_directories(jing PRIVATE ${mimalloc_SOURCE_DIR}/include)
# target_link_libraries(jing PRIVATE mimalloc-static)

# # build rpmalloc as a object library
# FetchContent_Declare(
#     rpmalloc
#     GIT_REPOSITORY https://github.com/mjansson/rpmalloc.git
#     GIT_TAG tags/1.4.5  
#     GIT_SHALLOW TRUE
# )
# message(STATUS "Configuring rpmalloc")
# FetchContent_MakeAvailable(rpmalloc)
# message(STATUS "Configuring rpmalloc succeed")
# add_library(rpmalloc_lib OBJECT ${rpmalloc_SOURCE_DIR}/rpmalloc/rpmalloc.c)
# target_compile_options(rpmalloc_lib PRIVATE ${JING_COMPILE_OPTIONS})
# target_include_directories(jing PRIVATE ${rpmalloc_SOURCE_DIR}/rpmalloc)
# target_link_libraries(jing PRIVATE $<TARGET_OBJECTS:rpmalloc_lib>)

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

# # build libdeflate as a static library
# FetchContent_Declare(
#   libdeflate
#   GIT_REPOSITORY https://github.com/ebiggers/libdeflate.git
#   GIT_TAG tags/v1.24
#   GIT_SHALLOW TRUE
# )
# set(LIBDEFLATE_BUILD_SHARED_LIB OFF CACHE BOOL "Whether to build libdeflate shared library" FORCE)
# set(LIBDEFLATE_BUILD_GZIP OFF CACHE BOOL "Whether to build libdeflate gzip console-program" FORCE)
# message(STATUS "Configuring libdeflate")
# FetchContent_MakeAvailable(libdeflate)
# message(STATUS "Configuring libdeflate succeed")
# target_compile_options(libdeflate_static PRIVATE ${JING_COMPILE_OPTIONS})
# target_include_directories(jing PRIVATE ${libdeflate_SOURCE_DIR})
# target_link_libraries(jing PRIVATE libdeflate_static)

# # build brotli as a static library
# FetchContent_Declare(
#     brotli
#     GIT_REPOSITORY https://github.com/google/brotli.git
#     GIT_TAG tags/v1.1.0
#     GIT_SHALLOW TRUE
# )
# set(BUILD_SHARED_LIBS OFF CACHE BOOL "Whether to build brotli shared library" FORCE)
# set(BUILD_TESTING OFF CACHE BOOL "Whether to build brotli tests" FORCE)
# message(STATUS "Configuring brotli")
# FetchContent_MakeAvailable(brotli)
# message(STATUS "Configuring brotli succeed")
# foreach(lib brotlicommon brotlienc brotlidec)
#     target_compile_options(${lib} PRIVATE ${JING_COMPILE_OPTIONS})
#     if(WIN32)
#         target_compile_options(${lib} PRIVATE /wd4334)
#     endif()
# endforeach()
# target_include_directories(jing PRIVATE ${brotli_SOURCE_DIR}/c/include)
# target_link_libraries(jing PRIVATE brotlicommon brotlienc brotlidec)

# build aws-lc
FetchContent_Declare(
    awslc
    GIT_REPOSITORY https://github.com/aws/aws-lc.git
    GIT_TAG tags/v1.60.0
    GIT_SHALLOW TRUE
)
set(BUILD_LIBSSL ON CACHE BOOL "Whether to build aws-lc libssl" FORCE)
set(BUILD_TESTING OFF CACHE BOOL "Whether to enable aws-lc testing" FORCE)
set(BUILD_TOOL OFF CACHE BOOL "Whether to build aws-lc tool" FORCE)
message(STATUS "Configuring aws-lc")
FetchContent_MakeAvailable(awslc)
message(STATUS "Configuring aws-lc succeed")
target_compile_options(crypto PRIVATE ${JING_COMPILE_OPTIONS})
target_compile_options(ssl PRIVATE ${JING_COMPILE_OPTIONS})
target_include_directories(jing PRIVATE ${awslc_SOURCE_DIR}/include)
target_link_libraries(jing PRIVATE crypto ssl)
# Add extra platform-specific dependencies for aws-lc
if(WIN32)
    target_link_libraries(jing PRIVATE ws2_32 crypt32 advapi32)
endif()

# build zstd
FetchContent_Declare(
    zstd
    GIT_REPOSITORY https://github.com/facebook/zstd.git
    GIT_TAG tags/v1.5.7
    GIT_SHALLOW TRUE
    SOURCE_SUBDIR build/cmake
)
set(ZSTD_BUILD_SHARED OFF CACHE BOOL "Whether to build zstd shared library" FORCE)
set(ZSTD_BUILD_STATIC ON CACHE BOOL "Whether to build zstd static library" FORCE)
set(ZSTD_BUILD_PROGRAMS OFF CACHE BOOL "Whether to build zstd program" FORCE)
set(ZSTD_BUILD_TESTS OFF CACHE BOOL "Whether to build zstd tests" FORCE)
set(ZSTD_BUILD_CONTRIB OFF CACHE BOOL "Whether to build zstd contrib" FORCE)
message(STATUS "Configuring zstd")
FetchContent_MakeAvailable(zstd)
message(STATUS "Configuring zstd succeed")
target_compile_options(libzstd_static PRIVATE ${JING_COMPILE_OPTIONS})
target_include_directories(jing PRIVATE ${zstd_SOURCE_DIR}/lib)
target_link_libraries(jing PRIVATE libzstd_static)