# =============================================================================
# hf-ws2812-rmt-driver — host unit tests
# =============================================================================
# The driver is ESP-IDF code. On a host it builds against the ESP-IDF header stubs in tests/stubs/
# (the root CMakeLists.txt adds them when this option is on), and fake_rmt.cpp records what the
# driver transmits instead of driving a pin. No hardware and no ESP-IDF needed:
#
#   cmake -S . -B build-host -DHF_WS2812_RMT_BUILD_TESTS=ON
#   cmake --build build-host
#   ctest --test-dir build-host --output-on-failure
# =============================================================================

add_library(hf_ws2812_rmt_fake_idf STATIC stubs/fake_rmt.cpp)
target_link_libraries(hf_ws2812_rmt_fake_idf PUBLIC ${HF_WS2812_RMT_TARGET_NAME})
target_include_directories(hf_ws2812_rmt_fake_idf PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/stubs)
target_compile_features(hf_ws2812_rmt_fake_idf PRIVATE cxx_std_20)

add_executable(hf_ws2812_rmt_host_tests
  test_main.cpp
  test_strip.cpp
  test_effects.cpp
  test_c_api.cpp
)

target_link_libraries(hf_ws2812_rmt_host_tests PRIVATE hf_ws2812_rmt_fake_idf)
target_include_directories(hf_ws2812_rmt_host_tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_features(hf_ws2812_rmt_host_tests PRIVATE cxx_std_20)
target_compile_options(hf_ws2812_rmt_host_tests PRIVATE -Wall -Wextra)

add_test(NAME hf_ws2812_rmt_host_tests COMMAND hf_ws2812_rmt_host_tests)

# The code shown on the documentation site (docs/ includes these files with pymdownx.snippets), built so a
# page cannot show code that does not compile against the current headers. Compiled, never linked or run.
add_library(hf_ws2812_rmt_doc_snippets OBJECT
  docs/quickstart.cpp
  docs/api_tasks.cpp
)
target_link_libraries(hf_ws2812_rmt_doc_snippets PRIVATE ${HF_WS2812_RMT_TARGET_NAME})
target_compile_features(hf_ws2812_rmt_doc_snippets PRIVATE cxx_std_20)
target_compile_options(hf_ws2812_rmt_doc_snippets PRIVATE -Wall -Wextra)
