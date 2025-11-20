#include "cap1166_light.h"
#include "esphome/core/log.h"
#include "esphome/core/hal.h"

namespace esphome {
namespace cap1166 {

static const char *const TAG = "cap1166.light";

void CAP1166Light::write_state(light::LightState *state)
 {
    bool binary;
    state->current_values_as_binary(&binary);
    if (binary) {
      this->parent_->turn_on(this->channel_);
    } else {
      this->parent_->turn_off(this->channel_);
    }
  }

void CAP1166Light::setup() {
  ESP_LOGD(TAG, "Configuring LED channel %d with behavior %d", this->channel_, this->led_behavior_);
  
  // Configure the LED behavior
  this->parent_->configure_led_behavior(this->channel_, this->led_behavior_);
  
  ESP_LOGD(TAG, "LED channel %d configured successfully", this->channel_);
}

}  // namespace cap1166
}  // namespace esphome
