#pragma once

#include "esphome/components/binary_sensor/binary_sensor.h"
#include "../cap1166.h"

namespace esphome {
namespace cap1166 {

class CAP1166BinarySensor : public binary_sensor::BinarySensor, public CAP1166Channel {
 public:
  void set_channel(uint8_t channel) { channel_ = channel; }
  void process(uint8_t data) { this->publish_state(static_cast<bool>(data & (1 << this->channel_))); }

 protected:
  uint8_t channel_{0};
};

}  // namespace cap1166
}  // namespace esphome
