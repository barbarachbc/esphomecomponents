#pragma once

#include "esphome/components/light/light_output.h"
#include "../cap1166.h"

namespace esphome {
namespace cap1166 {

class CAP1166Light : public light::LightOutput, public CAP1166LedChannel {
 public:
  void set_channel(uint8_t channel) { this->channel_ = channel; }
  uint8_t get_channel() { return this->channel_; }
  void set_led_behavior(CAP1166LedBehavior behavior) { this->led_behavior_ = behavior; }
  void set_link_to_touch(bool linked){ this->linked_to_touch_ = linked; }
  bool is_linked(){ return this->linked_to_touch_; }

  light::LightTraits get_traits() override {
    auto traits = light::LightTraits();
    traits.set_supported_color_modes({light::ColorMode::ON_OFF});
    return traits;
  }

  void write_state(light::LightState *state) override;
  void setup() override;

 protected:
  uint8_t channel_;
  CAP1166LedBehavior led_behavior_{LED_BEHAVIOR_DIRECT};
  bool linked_to_touch_;
};

}  // namespace cap1166
}  // namespace esphome
