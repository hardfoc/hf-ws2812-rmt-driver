/**
 * @file fake_rmt.hpp
 * @brief What the fake RMT driver recorded: the host tests read it to check the driver's output.
 */
#pragma once

#include "driver/rmt_tx.h"

#include <cstdint>
#include <vector>

namespace fake_rmt {

struct State {
  int channels_created = 0;
  int channels_deleted = 0;
  int transmits = 0;
  rmt_tx_channel_config_t last_tx_config{};
  /// Waveform of the last transmit: every symbol the encoder produced, in order (bytes encoders
  /// expanded to bit symbols, copy encoders as they are).
  std::vector<rmt_symbol_word_t> last_symbols;
  std::vector<uint8_t> last_bytes; ///< Bytes the last transmit fed to a bytes encoder.
  rmt_bytes_encoder_config_t last_bytes_config{};
  esp_err_t transmit_result = ESP_OK; ///< Returned by the next rmt_transmit() calls.
};

/** The recorder; reset() between tests. */
State& state();
void reset();

} // namespace fake_rmt
