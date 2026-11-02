# =============================================================================
# hf-ws2812-rmt-driver — Root CMakeLists.txt
# =============================================================================
# Dual-mode build file:
#   - ESP-IDF component (when ESP_PLATFORM is set, e.g. via idf_component.yml)
#   - Platform-agnostic static library (desktop / CI)
#
# Both paths import cmake/hf_ws2812_rmt_build_settings.cmake as the single
# source of truth for sources, includes, and version info.
# =============================================================================
cmake_minimum_required(VERSION 3.16)

include(${CMAKE_CURRENT_LIST_DIR}/cmake/hf_ws2812_rmt_build_settings.cmake)

# ── ESP-IDF path (used when installed via ESP Component Registry) ────────────
if(ESP_PLATFORM)
  idf_component_register(
      SRCS         ${HF_WS2812_RMT_SOURCE_FILES}
      INCLUDE_DIRS ${HF_WS2812_RMT_PUBLIC_INCLUDE_DIRS}
      REQUIRES     ${HF_WS2812_RMT_IDF_REQUIRES}
  )
  target_compile_options(${COMPONENT_LIB} PRIVATE
      $<$<COMPILE_LANGUAGE:CXX>:-std=gnu++20>)
  return()
endif()

# ── Platform-agnostic path ───────────────────────────────────────────────────
project(hf_ws2812_rmt
  VERSION   ${HF_WS2812_RMT_VERSION}
  LANGUAGES C CXX
)

include(GNUInstallDirs)
set(HF_WS2812_RMT_INSTALL_INCLUDEDIR "${CMAKE_INSTALL_INCLUDEDIR}/hf_ws2812_rmt")
set(HF_WS2812_RMT_INSTALL_CMAKEDIR "${CMAKE_INSTALL_LIBDIR}/cmake/hf_ws2812_rmt")

add_library(${HF_WS2812_RMT_TARGET_NAME} STATIC ${HF_WS2812_RMT_SOURCE_FILES})
add_library(hf::${HF_WS2812_RMT_TARGET_NAME} ALIAS ${HF_WS2812_RMT_TARGET_NAME})

foreach(dir IN LISTS HF_WS2812_RMT_PUBLIC_INCLUDE_DIRS)
  target_include_directories(${HF_WS2812_RMT_TARGET_NAME} PUBLIC $<BUILD_INTERFACE:${dir}>)
endforeach()
target_include_directories(${HF_WS2812_RMT_TARGET_NAME} PUBLIC
  $<INSTALL_INTERFACE:${HF_WS2812_RMT_INSTALL_INCLUDEDIR}>)

target_compile_features(${HF_WS2812_RMT_TARGET_NAME} PUBLIC cxx_std_20)

# ── Host unit tests (ESP-IDF headers stubbed; no hardware) ───────────────────
# The driver needs ESP-IDF. With the tests on, the library builds against the header stubs in tests/stubs/.
option(HF_WS2812_RMT_BUILD_TESTS "Build the host unit tests in tests/" OFF)
if(HF_WS2812_RMT_BUILD_TESTS)
  target_include_directories(${HF_WS2812_RMT_TARGET_NAME} BEFORE PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/tests/stubs>)
  enable_testing()
  add_subdirectory(tests)
endif()

# Optional warnings
option(HF_WS2812_RMT_ENABLE_WARNINGS "Enable compiler warnings" OFF)
if(HF_WS2812_RMT_ENABLE_WARNINGS)
  target_compile_options(${HF_WS2812_RMT_TARGET_NAME} PRIVATE -Wall -Wextra -Wpedantic)
endif()

# ── Install and CMake package export ─────────────────────────────────────────
# Installs the library, the public headers (and the generated ws2812_version.h) under
# include/hf_ws2812_rmt, and a package config: find_package(hf_ws2812_rmt) gives hf::hf_ws2812_rmt.
# The headers still need ESP-IDF (or the host stubs) to compile.
include(CMakePackageConfigHelpers)

install(TARGETS ${HF_WS2812_RMT_TARGET_NAME}
  EXPORT hf_ws2812_rmtTargets
  ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
file(GLOB HF_WS2812_RMT_PUBLIC_HEADERS
  "${CMAKE_CURRENT_SOURCE_DIR}/inc/*.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/inc/*.hpp")
install(FILES ${HF_WS2812_RMT_PUBLIC_HEADERS} "${HF_WS2812_RMT_VERSION_HEADER}"
  DESTINATION ${HF_WS2812_RMT_INSTALL_INCLUDEDIR})
install(EXPORT hf_ws2812_rmtTargets
  NAMESPACE hf::
  DESTINATION ${HF_WS2812_RMT_INSTALL_CMAKEDIR}
)

write_basic_package_version_file(
  "${CMAKE_CURRENT_BINARY_DIR}/hf_ws2812_rmtConfigVersion.cmake"
  VERSION ${PROJECT_VERSION}
  COMPATIBILITY SameMajorVersion
)

configure_package_config_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/cmake/hf_ws2812_rmtConfig.cmake.in"
  "${CMAKE_CURRENT_BINARY_DIR}/hf_ws2812_rmtConfig.cmake"
  INSTALL_DESTINATION ${HF_WS2812_RMT_INSTALL_CMAKEDIR}
)

install(FILES
  "${CMAKE_CURRENT_BINARY_DIR}/hf_ws2812_rmtConfig.cmake"
  "${CMAKE_CURRENT_BINARY_DIR}/hf_ws2812_rmtConfigVersion.cmake"
  DESTINATION ${HF_WS2812_RMT_INSTALL_CMAKEDIR}
)
