#include "tca9554.h"
#include "esphome/core/log.h"

static const uint8_t TCA9554_INPUT_PORT_REGISTER_0 = 0x00;
static const uint8_t TCA9554_OUTPUT_PORT_REGISTER_0 = 0x01;
static const uint8_t TCA9554_POLARITY_REGISTER_0 = 0x02;
static const uint8_t TCA9554_CONFIGURATION_PORT_0 = 0x03;

namespace esphome {
namespace tca9554 {

static const char *const TAG = "tca9554";

void TCA9554Component::setup() {
  if (!this->read_gpio_modes_()) {
    this->mark_failed();
    return;
  }
  if (!this->read_gpio_outputs_()) {
    this->mark_failed();
    return;
  }
}
void TCA9554Component::dump_config() {
  ESP_LOGCONFIG(TAG, "TCA9554:");
  LOG_I2C_DEVICE(this)
  if (this->is_failed()) {
    ESP_LOGE(TAG, ESP_LOG_MSG_COMM_FAIL);
  }
}
void TCA9554Component::pin_mode(uint8_t pin, gpio::Flags flags) {
  if (flags == gpio::FLAG_INPUT) {
    // Set mode mask bit
    this->mode_mask_ |= 1 << pin;
  } else if (flags == gpio::FLAG_OUTPUT) {
    // Clear mode mask bit
    this->mode_mask_ &= ~(1 << pin);
  }
  // Write GPIO to enable input mode
  this->write_gpio_modes_();
}
void TCA9554Component::loop() { this->reset_pin_cache_(); }

bool TCA9554Component::read_gpio_outputs_() {
  if (this->is_failed())
    return false;
  uint8_t data[1];
  if (!this->read_bytes(TCA9554_OUTPUT_PORT_REGISTER_0, data, 1)) {
    this->status_set_warning(LOG_STR("Failed to read output register"));
    return false;
  }
  this->output_mask_ = data[0];
  this->status_clear_warning();
  return true;
}

bool TCA9554Component::read_gpio_modes_() {
  if (this->is_failed())
    return false;
  uint8_t data[1];
  bool success = this->read_bytes(TCA9554_CONFIGURATION_PORT_0, data, 1);
  if (!success) {
    this->status_set_warning(LOG_STR("Failed to read mode register"));
    return false;
  }
  this->mode_mask_ = data[0];

  this->status_clear_warning();
  return true;
}
bool TCA9554Component::digital_read_hw(uint8_t pin) {
  if (this->is_failed())
    return false;
  uint8_t data;
  uint8_t register_to_read = TCA9554_INPUT_PORT_REGISTER_0;
  if (!this->read_bytes(register_to_read, &data, 1)) {
    this->status_set_warning(LOG_STR("Failed to read input register"));
    return false;
  }
  this->input_mask_ = data;

  this->status_clear_warning();
  return true;
}

void TCA9554Component::digital_write_hw(uint8_t pin, bool value) {
  if (this->is_failed())
    return;

  if (value) {
    this->output_mask_ |= (1 << pin);
  } else {
    this->output_mask_ &= ~(1 << pin);
  }

  uint8_t data[1];
  data[0] = this->output_mask_;
  if (!this->write_bytes(TCA9554_OUTPUT_PORT_REGISTER_0, data, 1)) {
    this->status_set_warning(LOG_STR("Failed to write output register"));
    return;
  }

  this->status_clear_warning();
}

bool TCA9554Component::write_gpio_modes_() {
  if (this->is_failed())
    return false;
  uint8_t data[1];

  data[0] = this->mode_mask_;
  if (!this->write_bytes(TCA9554_CONFIGURATION_PORT_0, data, 1)) {
    this->status_set_warning(LOG_STR("Failed to write mode register"));
    return false;
  }
  this->status_clear_warning();
  return true;
}

bool TCA9554Component::digital_read_cache(uint8_t pin) { return this->input_mask_ & (1 << pin); }

float TCA9554Component::get_setup_priority() const { return setup_priority::IO; }

void TCA9554GPIOPin::setup() { this->pin_mode(this->flags_); }
void TCA9554GPIOPin::pin_mode(gpio::Flags flags) { this->parent_->pin_mode(this->pin_, flags); }
bool TCA9554GPIOPin::digital_read() { return this->parent_->digital_read(this->pin_) != this->inverted_; }
void TCA9554GPIOPin::digital_write(bool value) { this->parent_->digital_write(this->pin_, value != this->inverted_); }
std::string TCA9554GPIOPin::dump_summary() const { return str_sprintf("%u via TCA9554", this->pin_); }

}  // namespace tca9554
}  // namespace esphome
