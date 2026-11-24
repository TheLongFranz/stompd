# cmake/cm5-sdk.cmake — use the installed SDK without sourcing it
if(DEFINED ENV{STOMPD_SDK})
  set(_sdk "$ENV{STOMPD_SDK}")
else()
  set(_sdk "$ENV{HOME}/sdk/stompd")
endif()

file(GLOB _oe_toolchain "${_sdk}/sysroots/*sdk-linux/usr/share/cmake/*-toolchain.cmake")
if(NOT _oe_toolchain)
  message(FATAL_ERROR "No installed SDK found in ${_sdk}")
endif()
include("${_oe_toolchain}")

# Load the snippets other layers add for SDK builds (meta-qt6 sets QT_HOST_PATH here)
file(GLOB _oe_snippets "${HOST_SYSROOT}/usr/share/cmake/OEToolchainConfig.cmake.d/*.cmake")
foreach(_snippet IN LISTS _oe_snippets)
    include("${_snippet}")
endforeach()

# Swap the SDK's GCC for its Clang. GCC_DIR is the SDK folder with all the
# target-prefixed tools, including aarch64-poky-linux-clang++.
set(CMAKE_C_COMPILER   "${GCC_DIR}/${TOOLCHAIN_NAME}-clang")
set(CMAKE_CXX_COMPILER "${GCC_DIR}/${TOOLCHAIN_NAME}-clang++")
set(CMAKE_LINKER_TYPE LLD)
set(CMAKE_CXX_COMPILER_CLANG_SCAN_DEPS "${HOST_BIN}/clang-scan-deps" CACHE FILEPATH "")

# Only matters if you use pkg_check_modules
set(ENV{PKG_CONFIG_SYSROOT_DIR} "${CMAKE_SYSROOT}")
set(ENV{PKG_CONFIG_LIBDIR} "${CMAKE_SYSROOT}/usr/lib/pkgconfig:${CMAKE_SYSROOT}/usr/share/pkgconfig")
