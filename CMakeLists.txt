cmake_minimum_required(VERSION 3.24...4.0)

# set c standard
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

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

# set compiler toolchain
if(WIN32)
    set(CMAKE_GENERATOR_PLATFORM "x64" CACHE STRING "" FORCE)
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

# build using this command : cmake -E remove_directory build && cmake -B build -DCMAKE_BUILD_TYPE=Release && cmake --build build --config Release --verbose > cmake.log

# use release build by default
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Build type" FORCE)
endif()

# add support for fetch series
include(FetchContent)

# check for windows environment 
if(WIN32)
    add_compile_options(/wd4819)
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreadedDLL" CACHE STRING "" FORCE)
    execute_process(
        COMMAND nasm -v
        RESULT_VARIABLE NASM_RESULT
        OUTPUT_QUIET
        ERROR_QUIET
    )
    if(NASM_RESULT EQUAL 0)
        message(STATUS "Nasm found")
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

# # enable LTO
# include(CheckIPOSupported)
# check_ipo_supported()
# set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)

# our target
add_library(jing SHARED 
    src/jing_common.c 
    src/jing_buffer.c 
    src/jing_fastfloat.cpp 
    src/jing_malloc.c 
    src/jing_rpmalloc.c 
    src/jing_mimalloc.c
    src/jing_deflate.c
    src/jing_brotli.c
    src/jing_zstd.c
    src/jing_ssl.c
)

# compiler options for our target
if(WIN32)
    set(JING_COMPILE_OPTIONS "/MD;/O2;/W3;/GF;/nologo")
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

# build wepoll as a object library
if(WIN32)
    FetchContent_Declare(
        wepoll
        GIT_REPOSITORY https://github.com/piscisaureus/wepoll.git
        GIT_TAG tags/v1.5.8
        GIT_SHALLOW TRUE
    )
    message(STATUS "Configuring wepoll")
    FetchContent_MakeAvailable(wepoll)
    message(STATUS "Configuring wepoll succeed")
    add_library(wepoll_lib OBJECT ${wepoll_SOURCE_DIR}/wepoll.c)
    target_compile_options(wepoll_lib PRIVATE ${JING_COMPILE_OPTIONS})
    target_include_directories(jing PRIVATE ${wepoll_SOURCE_DIR})
    target_link_libraries(jing PRIVATE ws2_32 $<TARGET_OBJECTS:wepoll_lib>)
endif()

# build mimalloc as a static library
FetchContent_Declare(
    mimalloc
    GIT_REPOSITORY https://github.com/microsoft/mimalloc.git
    GIT_TAG tags/v3.1.5 
    GIT_SHALLOW TRUE
)
set(MI_BUILD_SHARED OFF CACHE BOOL "Whether to build mimalloc shared library" FORCE)
set(MI_BUILD_STATIC ON CACHE BOOL  "Whether to build mimalloc static library" FORCE)
set(MI_BUILD_OBJECT OFF CACHE BOOL "Whether to build mimalloc object library" FORCE)
set(MI_BUILD_TESTS OFF CACHE BOOL  "Whether to build mimalloc tests" FORCE)
set(MI_OVERRIDE OFF CACHE BOOL "Whether to override malloc and free" FORCE)
set(MI_OPT_ARCH ON CACHE BOOL "Whether to enable architecture specific optimizations" FORCE)
set(MI_OPT_SIMD ON CACHE BOOL "Whether to enable SIMD optimizations" FORCE)
message(STATUS "Configuring mimalloc")
FetchContent_MakeAvailable(mimalloc)
message(STATUS "Configuring mimalloc succeed")
target_compile_options(mimalloc-static PRIVATE ${JING_COMPILE_OPTIONS})
target_include_directories(jing PRIVATE ${mimalloc_SOURCE_DIR}/include)
target_link_libraries(jing PRIVATE mimalloc-static)

# build rpmalloc as a object library
FetchContent_Declare(
    rpmalloc
    GIT_REPOSITORY https://github.com/mjansson/rpmalloc.git
    GIT_TAG tags/1.4.5  
    GIT_SHALLOW TRUE
)
message(STATUS "Configuring rpmalloc")
FetchContent_MakeAvailable(rpmalloc)
message(STATUS "Configuring rpmalloc succeed")
add_library(rpmalloc_lib OBJECT ${rpmalloc_SOURCE_DIR}/rpmalloc/rpmalloc.c)
target_compile_options(rpmalloc_lib PRIVATE ${JING_COMPILE_OPTIONS})
target_include_directories(jing PRIVATE ${rpmalloc_SOURCE_DIR}/rpmalloc)
target_link_libraries(jing PRIVATE $<TARGET_OBJECTS:rpmalloc_lib>)

# build fastfloat as a header-only library
FetchContent_Declare(
  fast_float
  GIT_REPOSITORY https://github.com/fastfloat/fast_float.git
  GIT_TAG tags/v8.0.2
  GIT_SHALLOW TRUE
)
message(STATUS "Configuring fast_float")
FetchContent_MakeAvailable(fast_float)
message(STATUS "Configuring fast_float succeed")
target_include_directories(jing PRIVATE ${fast_float_SOURCE_DIR}/include)

# build libdeflate as a static library
FetchContent_Declare(
  libdeflate
  GIT_REPOSITORY https://github.com/ebiggers/libdeflate.git
  GIT_TAG tags/v1.24
  GIT_SHALLOW TRUE
)
set(LIBDEFLATE_BUILD_SHARED_LIB OFF CACHE BOOL "Whether to build libdeflate shared library" FORCE)
set(LIBDEFLATE_BUILD_GZIP OFF CACHE BOOL "Whether to build libdeflate gzip console-program" FORCE)
message(STATUS "Configuring libdeflate")
FetchContent_MakeAvailable(libdeflate)
message(STATUS "Configuring libdeflate succeed")
target_compile_options(libdeflate_static PRIVATE ${JING_COMPILE_OPTIONS})
target_include_directories(jing PRIVATE ${libdeflate_SOURCE_DIR})
target_link_libraries(jing PRIVATE libdeflate_static)

# build brotli as a static library
FetchContent_Declare(
    brotli
    GIT_REPOSITORY https://github.com/google/brotli.git
    GIT_TAG tags/v1.1.0
    GIT_SHALLOW TRUE
)
set(BUILD_SHARED_LIBS OFF CACHE BOOL "Whether to build brotli shared library" FORCE)
set(BUILD_TESTING OFF CACHE BOOL "Whether to build brotli tests" FORCE)
message(STATUS "Configuring brotli")
FetchContent_MakeAvailable(brotli)
message(STATUS "Configuring brotli succeed")
foreach(lib brotlicommon brotlienc brotlidec)
    target_compile_options(${lib} PRIVATE ${JING_COMPILE_OPTIONS})
    if(WIN32)
        target_compile_options(${lib} PRIVATE /wd4334)
    endif()
endforeach()
target_include_directories(jing PRIVATE ${brotli_SOURCE_DIR}/c/include)
target_link_libraries(jing PRIVATE brotlicommon brotlienc brotlidec)

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

# build openssl
FetchContent_Declare(
    openssl
    URL https://github.com/openssl/openssl/releases/download/openssl-3.5.1/openssl-3.5.1.tar.gz
    URL_HASH SHA256=529043b15cffa5f36077a4d0af83f3de399807181d607441d734196d889b641f
)
message(STATUS "Configuring openssl")
FetchContent_MakeAvailable(openssl)
message(STATUS "Configuring openssl succeed")
target_include_directories(jing PRIVATE ${openssl_SOURCE_DIR}/include)
if(WIN32)
    message(STATUS "Configuring OpenSSL for Windows")
    file(WRITE "${CMAKE_BINARY_DIR}/jing_build_openssl.bat" "
    @echo off
    call \"${VCVARS_PATH}\"
    cd \"${openssl_SOURCE_DIR}\"
    perl Configure VC-WIN64A --release ^
        --with-brotli-include=\"${brotli_SOURCE_DIR}/c/include\" ^
        --with-brotli-lib=\"${brotli_BINARY_DIR}/Release\" ^
        --with-zstd-include=\"${zstd_SOURCE_DIR}/lib\" ^
        --with-zstd-lib=\"${zstd_BINARY_DIR}/lib/Release/zstd_static.lib\" ^
        no-apps enable-brotli enable-zstd no-deprecated no-docs ^
        no-tls-deprecated-ec no-slh-dsa no-sm2-precomp no-srtp no-tests ^
        no-ui-console no-stdio no-uplink no-ssl3 no-tls1 no-tls1_1 no-dtls1 ^
        no-ssl3-method no-tls1-method no-tls1_1-method no-dtls1-method ^
        no-des no-rc4 no-rc2 no-md4 no-rmd160 no-idea no-seed no-bf no-mdc2 ^
        CFLAGS=\"/MD /O2 /W3 /GF /nologo /wd4090 /wd4133 /wd4267 /wd4244 /wd4334 /wd4819\"
    nmake
    ")
    add_custom_target(build_openssl
        COMMAND cmd.exe /c "${CMAKE_BINARY_DIR}/jing_build_openssl.bat"
        WORKING_DIRECTORY ${openssl_SOURCE_DIR}
        VERBATIM
    )
    target_link_libraries(jing PRIVATE ws2_32 crypt32 advapi32 ${openssl_SOURCE_DIR}/libcrypto.lib ${openssl_SOURCE_DIR}/libssl.lib brotlicommon brotlienc brotlidec libzstd_static)
elseif(APPLE)
    message(STATUS "Configuring OpenSSL for macOS")
    file(WRITE "${CMAKE_BINARY_DIR}/jing_build_openssl.sh" "
    #!/bin/sh
    cd \"${openssl_SOURCE_DIR}\"
    ./Configure darwin64-arm64 --release \\
        --with-brotli-include=${brotli_SOURCE_DIR}/c/include \\
        --with-brotli-lib=${brotli_BINARY_DIR}/Release \\
        --with-zstd-include=${zstd_SOURCE_DIR}/lib \\
        --with-zstd-lib=${zstd_BINARY_DIR}/lib/Release \\
        no-apps enable-brotli enable-zstd no-deprecated no-docs \\
        no-tls-deprecated-ec no-slh-dsa no-sm2-precomp \\
        no-srtp no-tests no-ui-console no-stdio enable-tfo no-uplink \\
        no-ssl3 no-tls1 no-tls1_1 no-dtls1 no-ssl3-method \\
        no-tls1-method no-tls1_1-method no-dtls1-method \\
        no-des no-rc4 no-rc2 no-md4 no-rmd160 no-idea no-seed no-bf no-mdc2 \\
        CFLAGS=\"-O3 -Wall -fvisibility=hidden\"
    make -j4
    ")
    add_custom_target(build_openssl
        COMMAND chmod +x "${CMAKE_BINARY_DIR}/jing_build_openssl.sh"
        COMMAND "${CMAKE_BINARY_DIR}/jing_build_openssl.sh"
        WORKING_DIRECTORY ${openssl_SOURCE_DIR}
        VERBATIM
    )
    target_link_libraries(jing PRIVATE ${openssl_SOURCE_DIR}/libcrypto.dylib ${openssl_SOURCE_DIR}/libssl.dylib brotlicommon brotlienc brotlidec libzstd_static)
elseif(UNIX)
    message(STATUS "Configuring OpenSSL for linux")
    file(WRITE "${CMAKE_BINARY_DIR}/jing_build_openssl.sh" "
    #!/bin/sh
    cd \"${openssl_SOURCE_DIR}\"
    ./Configure linux-x86_64 --release \\
        --with-brotli-include=${brotli_SOURCE_DIR}/c/include \\
        --with-brotli-lib=${brotli_BINARY_DIR}/Release \\
        --with-zstd-include=${zstd_SOURCE_DIR}/lib \\
        --with-zstd-lib=${zstd_BINARY_DIR}/lib/Release \\
        enable-ktls no-apps enable-brotli enable-zstd no-deprecated no-docs \\
        no-tls-deprecated-ec enable-ec_nistp_64_gcc_128 no-shared no-slh-dsa \\
        no-sm2-precomp no-srtp no-tests no-ui-console no-stdio enable-tfo no-uplink \\
        no-ssl3 no-tls1 no-tls1_1 no-dtls1 no-ssl3-method no-tls1-method \\
        no-tls1_1-method no-dtls1-method no-des no-rc4 no-rc2 no-md4 \\
        no-rmd160 no-idea no-seed no-bf no-mdc2 \\
        CFLAGS=\"-O3 -Wall -fvisibility=hidden\"
    make -j4
    ")
    add_custom_target(build_openssl
        COMMAND chmod +x "${CMAKE_BINARY_DIR}/jing_build_openssl.sh"
        COMMAND "${CMAKE_BINARY_DIR}/jing_build_openssl.sh"
        WORKING_DIRECTORY ${openssl_SOURCE_DIR}
        VERBATIM
    )
    target_link_libraries(jing PRIVATE ${openssl_SOURCE_DIR}/libcrypto.a ${openssl_SOURCE_DIR}/libssl.a brotlicommon brotlienc brotlidec libzstd_static)
else()
    message(FATAL_ERROR "Unrecognized os")
endif()

add_dependencies(build_openssl brotlicommon brotlienc brotlidec libzstd_static)
add_dependencies(jing build_openssl)