# Consumer check for the installed package (CI: host-tests workflow, "Install" step):
#   cmake --install build-host --prefix build-host/_install
#   cmake -S tests/install -B build-host/_consumer -DCMAKE_PREFIX_PATH=$PWD/build-host/_install
cmake_minimum_required(VERSION 3.16)
project(hf_ws2812_rmt_install_check LANGUAGES NONE)

find_package(hf_ws2812_rmt 1 REQUIRED CONFIG)
if(NOT TARGET hf::hf_ws2812_rmt)
  message(FATAL_ERROR "hf::hf_ws2812_rmt not defined by the installed package")
endif()
get_target_property(inc hf::hf_ws2812_rmt INTERFACE_INCLUDE_DIRECTORIES)
foreach(header ws2812_cpp.hpp ws2812_control.h ws2812_rmt_caps.h ws2812_version.h rmt_wrapper.hpp)
  if(NOT EXISTS "${inc}/${header}")
    message(FATAL_ERROR "installed package lacks ${header} in ${inc}")
  endif()
endforeach()
message(STATUS "hf_ws2812_rmt ${hf_ws2812_rmt_VERSION} installed correctly")
