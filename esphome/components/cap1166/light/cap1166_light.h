#pragma once

#include "esphome/components/light/light_output.h"
#include "../cap1166.h"

namespace esphome {
namespace cap1166 {

class CAP1166Light : public light::LightOutput, public CAP1166LedChannel {
 public:
  void set_channel(uint8_t channel) { channel_ = channel; }
  uint8_t get_channel() { return channel_; }
  void set_led_behavior(CAP1166LedBehavior behavior) { led_behavior_ = behavior; }

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
};

}  // namespace cap1166
}  // namespace esphome
