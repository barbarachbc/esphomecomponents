#include "cap1166.h"
#include "esphome/core/log.h"
#include "esphome/core/hal.h"

namespace esphome {
namespace cap1166 {

static const char *const TAG = "cap1166";

void CAP1166Component::setup() {
  this->disable_loop();

  // no reset pin
  if (this->reset_pin_ == nullptr) {
    this->finish_setup_();
    return;
  }

  // reset pin configured so reset before finishing setup
  this->reset_pin_->setup();
  this->reset_pin_->digital_write(false);
  // delay after reset pin write
  this->set_timeout(100, [this]() {
    this->reset_pin_->digital_write(true);
    // delay after reset pin write
    this->set_timeout(100, [this]() {
      this->reset_pin_->digital_write(false);
      // delay after reset pin write
      this->set_timeout(100, [this]() { this->finish_setup_(); });
    });
  });
}

void CAP1166Component::finish_setup_() {
  // Check if CAP1166 is actually connected
  this->read_byte(CAP1166_PRODUCT_ID, &this->cap1166_product_id_);
  this->read_byte(CAP1166_MANUFACTURE_ID, &this->cap1166_manufacture_id_);
  this->read_byte(CAP1166_REVISION, &this->cap1166_revision_);

  if ((this->cap1166_product_id_ != 0x51) || (this->cap1166_manufacture_id_ != 0x5D)) {
    this->error_code_ = COMMUNICATION_FAILED;
    this->mark_failed();
    return;
  }

  // Set sensitivity
  uint8_t sensitivity = 0;
  this->read_byte(CAP1166_SENSITIVITY, &sensitivity);
  sensitivity = sensitivity & 0x0f;
  this->write_byte(CAP1166_SENSITIVITY, sensitivity | this->touch_threshold_);

  // Allow multiple touches
  this->write_byte(CAP1166_MULTI_TOUCH, this->allow_multiple_touches_);

  // Have LEDs follow touches
  //if not linked - unlink all, otherwise unlink those configured as lights
  this->write_byte(CAP1166_LED_LINK, (this->link_leds_ & ~(this->led_channels_mask_)));
  // Speed up a bit
  this->write_byte(CAP1166_STAND_BY_CONFIGURATION, 0x30);

  for (auto *channel : this->led_channels_) {
    channel->setup();
  }

  // Setup successful, so enable loop
  this->enable_loop();
}

void CAP1166Component::dump_config() {
  ESP_LOGCONFIG(TAG, "CAP1166:");
  LOG_I2C_DEVICE(this);
  LOG_PIN("  Reset Pin: ", this->reset_pin_);
  ESP_LOGCONFIG(TAG,
                "  Product ID: 0x%x\n"
                "  Manufacture ID: 0x%x\n"
                "  Revision ID: 0x%x",
                this->cap1166_product_id_, this->cap1166_manufacture_id_, this->cap1166_revision_);

  switch (this->error_code_) {
    case COMMUNICATION_FAILED:
      ESP_LOGE(TAG, "Product ID or Manufacture ID of the connected device does not match a known CAP1166.");
      break;
    case NONE:
    default:
      break;
  }
}

void CAP1166Component::loop() {
  uint8_t touched = 0;

  this->read_register(CAP1166_SENSOR_INPUT_STATUS, &touched, 1);

  if (touched) {
    uint8_t data = 0;
    this->read_register(CAP1166_MAIN, &data, 1);
    data = data & ~CAP1166_MAIN_INT;

    this->write_byte(CAP1166_MAIN, data);
  }

  for (auto *channel : this->channels_) {
    channel->process(touched);
  }
}

void CAP1166Component::turn_on(uint8_t channel) {
  uint8_t data = 0;
  this->read_register(CAP1166_LED_OUT, &data, 1);
  ESP_LOGD(TAG, "Turning ON channel - %01u, registry value: %01u", channel, data);
  data = data | (1 << channel);
  ESP_LOGV(TAG, "Turning ON channel - %01u, registry value: %01u", channel, data);
  this->write_register(CAP1166_LED_OUT, &data, 1);
}

void CAP1166Component::turn_off(uint8_t channel) {
  uint8_t data = 0;
  this->read_register(CAP1166_LED_OUT, &data, 1);
  ESP_LOGD(TAG, "Turning OFF channel - %01u, registry value: %01u", channel, data);
  data = data & (~(1 << channel));
  ESP_LOGV(TAG, "Turning OFF channel - %01u, registry value: %01u", channel, data);
  this->write_register(CAP1166_LED_OUT, &data, 1);
}

void CAP1166Component::register_channel(CAP1166LedChannel *channel) {
  if(!channel->is_linked())
  {
    this->led_channels_mask_ |= (1 << channel->get_channel());
  }
  this->led_channels_.push_back(channel);
  channel->set_parent(this);
  ESP_LOGD(TAG, "Registered channel: %01u", channel->get_channel());
}

void CAP1166Component::configure_led_behavior(uint8_t channel, CAP1166LedBehavior behavior) {
  // Configure LED behavior in behavior registers
  uint8_t behavior_reg = (channel < 4) ? CAP1166_LED_BEHAVIOUR1 : CAP1166_LED_BEHAVIOUR2;
  uint8_t shift = (channel % 4) * 2;
  
  uint8_t reg_value = 0;
  this->read_byte(behavior_reg, &reg_value);
  
  // Clear the 2 bits for this channel
  reg_value &= ~(0x03 << shift);
  // Set the new behavior
  reg_value |= (behavior << shift);
  
  this->write_byte(behavior_reg, reg_value);
  
  ESP_LOGD(TAG, "Configured LED behavior for channel %d: %d (reg 0x%02x = 0x%02x)", 
           channel, behavior, behavior_reg, reg_value);
}

}  // namespace cap1166
}  // namespace esphome
