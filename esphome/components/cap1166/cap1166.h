#pragma once

#include "esphome/core/component.h"
#include "esphome/core/hal.h"
#include "esphome/components/i2c/i2c.h"
#include "esphome/components/output/binary_output.h"
#include "esphome/components/binary_sensor/binary_sensor.h"

#include <vector>

namespace esphome {
namespace cap1166 {

enum {
  CAP1166_I2CADDR = 0x29,
  CAP1166_SENSOR_INPUT_STATUS = 0x3,
  CAP1166_MULTI_TOUCH = 0x2A,
  CAP1166_LED_LINK = 0x72,
  CAP1166_PRODUCT_ID = 0xFD,
  CAP1166_MANUFACTURE_ID = 0xFE,
  CAP1166_STAND_BY_CONFIGURATION = 0x41,
  CAP1166_REVISION = 0xFF,
  CAP1166_MAIN = 0x00,
  CAP1166_MAIN_INT = 0x01,
  CAP1166_LEDPOL = 0x73,
  CAP1166_INTERUPT_REPEAT = 0x28,
  CAP1166_SENSITVITY = 0x1f,
};

class CAP1166Channel : public binary_sensor::BinarySensor {
 public:
  void set_channel(uint8_t channel) { channel_ = channel; }
  void process(uint8_t data) { this->publish_state(static_cast<bool>(data & (1 << this->channel_))); }

 protected:
  uint8_t channel_{0};
};

class CAP1166Component : public Component, public i2c::I2CDevice {
 public:
  void register_channel(CAP1166Channel *channel) { this->channels_.push_back(channel); }
  void set_touch_threshold(uint8_t touch_threshold) { this->touch_threshold_ = touch_threshold; };
  void set_allow_multiple_touches(bool allow_multiple_touches) {
    this->allow_multiple_touches_ = allow_multiple_touches ? 0x41 : 0x80;
  };
  void set_reset_pin(GPIOPin *reset_pin) { this->reset_pin_ = reset_pin; }
  void setup() override;
  void dump_config() override;
  void loop() override;

 protected:
  void finish_setup_();

  std::vector<CAP1166Channel *> channels_{};
  uint8_t touch_threshold_{0x20};
  uint8_t allow_multiple_touches_{0x80};

  GPIOPin *reset_pin_{nullptr};

  uint8_t cap1166_product_id_{0};
  uint8_t cap1166_manufacture_id_{0};
  uint8_t cap1166_revision_{0};

  enum ErrorCode {
    NONE = 0,
    COMMUNICATION_FAILED,
  } error_code_{NONE};
};

}  // namespace cap1166
}  // namespace esphome
