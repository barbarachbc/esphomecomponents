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
  this->read_byte(CAP1166_SENSITVITY, &sensitivity);
  sensitivity = sensitivity & 0x0f;
  this->write_byte(CAP1166_SENSITVITY, sensitivity | this->touch_threshold_);

  // Allow multiple touches
  this->write_byte(CAP1166_MULTI_TOUCH, this->allow_multiple_touches_);

  // Have LEDs follow touches
  this->write_byte(CAP1166_LED_LINK, 0xFF);

  // Speed up a bit
  this->write_byte(CAP1166_STAND_BY_CONFIGURATION, 0x30);

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

    this->write_register(CAP1166_MAIN, &data, 2);
  }

  for (auto *channel : this->channels_) {
    channel->process(touched);
  }
}

}  // namespace cap1166
}  // namespace esphome
