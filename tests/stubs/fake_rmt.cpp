/**
 * @file fake_rmt.cpp
 * @brief Host implementation of the ESP-IDF RMT, queue and task calls the driver uses: it records
 * what the driver transmits instead of driving a pin.
 */
#include "fake_rmt.hpp"

#include "driver/rmt_rx.h"
#include "freertos/queue.h"
#include "freertos/task.h"

#include <cstdlib>
#include <cstring>

struct rmt_channel_t {
  int id;
};

namespace {

enum class EncoderKind { Copy, Bytes };

struct FakeEncoder {
  rmt_encoder_t base; // first member, so the handle converts back
  EncoderKind kind;
  rmt_bytes_encoder_config_t bytes;
};

} // namespace

namespace fake_rmt {

State& state() {
  static State s;
  return s;
}

void reset() {
  state() = State{};
}

} // namespace fake_rmt

extern "C" {

const char* esp_err_to_name(esp_err_t code) {
  return code == ESP_OK ? "ESP_OK" : "ESP_FAIL";
}

esp_err_t rmt_new_tx_channel(const rmt_tx_channel_config_t* config,
                             rmt_channel_handle_t* ret_chan) {
  if (config == nullptr || ret_chan == nullptr) {
    return ESP_ERR_INVALID_ARG;
  }
  auto& s = fake_rmt::state();
  s.last_tx_config = *config;
  *ret_chan = new rmt_channel_t{s.channels_created++};
  return ESP_OK;
}

esp_err_t rmt_new_rx_channel(const rmt_rx_channel_config_t* config,
                             rmt_channel_handle_t* ret_chan) {
  if (config == nullptr || ret_chan == nullptr) {
    return ESP_ERR_INVALID_ARG;
  }
  *ret_chan = new rmt_channel_t{fake_rmt::state().channels_created++};
  return ESP_OK;
}

esp_err_t rmt_enable(rmt_channel_handle_t /*channel*/) {
  return ESP_OK;
}

esp_err_t rmt_disable(rmt_channel_handle_t /*channel*/) {
  return ESP_OK;
}

esp_err_t rmt_del_channel(rmt_channel_handle_t channel) {
  delete channel;
  ++fake_rmt::state().channels_deleted;
  return ESP_OK;
}

esp_err_t rmt_new_copy_encoder(const rmt_copy_encoder_config_t* /*config*/,
                               rmt_encoder_handle_t* ret_encoder) {
  auto* enc = new FakeEncoder{};
  enc->kind = EncoderKind::Copy;
  *ret_encoder = &enc->base;
  return ESP_OK;
}

esp_err_t rmt_new_bytes_encoder(const rmt_bytes_encoder_config_t* config,
                                rmt_encoder_handle_t* ret_encoder) {
  auto* enc = new FakeEncoder{};
  enc->kind = EncoderKind::Bytes;
  enc->bytes = *config;
  *ret_encoder = &enc->base;
  return ESP_OK;
}

esp_err_t rmt_del_encoder(rmt_encoder_handle_t encoder) {
  delete reinterpret_cast<FakeEncoder*>(encoder);
  return ESP_OK;
}

esp_err_t rmt_encoder_reset(rmt_encoder_handle_t /*encoder*/) {
  return ESP_OK;
}

void* rmt_alloc_encoder_mem(size_t size) {
  return std::calloc(1, size);
}

esp_err_t rmt_transmit(rmt_channel_handle_t tx_channel, rmt_encoder_handle_t encoder,
                       const void* payload, size_t payload_bytes,
                       const rmt_transmit_config_t* /*config*/) {
  auto& s = fake_rmt::state();
  if (tx_channel == nullptr || encoder == nullptr || payload == nullptr) {
    return ESP_ERR_INVALID_ARG;
  }
  if (s.transmit_result != ESP_OK) {
    return s.transmit_result;
  }
  ++s.transmits;
  const auto* enc = reinterpret_cast<const FakeEncoder*>(encoder);
  if (enc->kind == EncoderKind::Copy) {
    s.last_symbols.resize(payload_bytes / sizeof(rmt_symbol_word_t));
    std::memcpy(s.last_symbols.data(), payload, s.last_symbols.size() * sizeof(rmt_symbol_word_t));
  } else {
    const auto* bytes = static_cast<const uint8_t*>(payload);
    s.last_bytes.assign(bytes, bytes + payload_bytes);
    s.last_bytes_config = enc->bytes;
  }
  return ESP_OK;
}

esp_err_t rmt_tx_wait_all_done(rmt_channel_handle_t /*tx_channel*/, int /*timeout_ms*/) {
  return ESP_OK;
}

esp_err_t rmt_rx_register_event_callbacks(rmt_channel_handle_t /*rx_channel*/,
                                          const rmt_rx_event_callbacks_t* /*cbs*/,
                                          void* /*user_data*/) {
  return ESP_OK;
}

esp_err_t rmt_receive(rmt_channel_handle_t /*rx_channel*/, void* /*buffer*/, size_t /*buffer_size*/,
                      const rmt_receive_config_t* /*config*/) {
  return ESP_OK;
}

QueueHandle_t xQueueCreate(uint32_t /*length*/, uint32_t /*item_size*/) {
  return nullptr;
}

BaseType_t xQueueReceive(QueueHandle_t /*queue*/, void* /*item*/, TickType_t /*wait*/) {
  return pdFAIL;
}

BaseType_t xQueueSendFromISR(QueueHandle_t /*queue*/, const void* /*item*/, BaseType_t* /*woken*/) {
  return pdPASS;
}

void vQueueDelete(QueueHandle_t /*queue*/) {}

void vTaskDelay(TickType_t /*ticks*/) {}

} // extern "C"
