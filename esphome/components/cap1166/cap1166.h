#pragma once

#include "esphome/core/component.h"
#include "esphome/core/hal.h"
#include "esphome/components/i2c/i2c.h"
#include "esphome/components/output/binary_output.h"
#include "esphome/components/binary_sensor/binary_sensor.h"
#include "esphome/components/light/light_output.h"

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
  CAP1166_INTERUPT_REPEAT = 0x28,
  CAP1166_SENSITIVITY = 0x1f,
  CAP1166_LEDPOL = 0x73,
  CAP1166_LED_OUT = 0x74, //The LED Output Control Register controls the output state of the LED pins that are not linked to sensor inputs
  CAP1166_LED_BEHAVIOUR1 = 0x81, //LEDs 1-4; Each led has 2 bits defining behaviour: 
  CAP1166_LED_BEHAVIOUR2 = 0x82, //LEDs 5-6; 00 - direct, 01 - pulse 1, 10 - pulse 2, 11 - breathe
  /*
  The LED Duty Cycle registers determine the minimum and maximum duty cycle settings used for the LED for each LED
  behavior. These settings affect the brightness of the LED when it is fully off and fully on.
  The LED driver duty cycle will ramp up from the minimum duty cycle to the maximum duty cycle and back down again.
  */
  CAP1166_LED_DUTY_PULSE1 = 0x90,
  CAP1166_LED_DUTY_PULSE2 = 0x91,
  CAP1166_LED_DUTY_BREATH = 0x92,
  CAP1166_LED_DUTY_DIRECT = 0x93,
};

// LED behavior enumeration
enum CAP1166LedBehavior {
  LED_BEHAVIOR_DIRECT = 0x00,   // 00 - direct
  LED_BEHAVIOR_PULSE1 = 0x01,   // 01 - pulse 1
  LED_BEHAVIOR_PULSE2 = 0x02,   // 10 - pulse 2  
  LED_BEHAVIOR_BREATHE = 0x03,  // 11 - breathe
};

class CAP1166Channel {
 public:
  virtual void process(uint8_t data) = 0;
};

class CAP1166Component;

class CAP1166LedChannel : public Parented<CAP1166Component> {
  public:
    virtual uint8_t get_channel() = 0;
    virtual bool is_linked() = 0;
    virtual void setup() = 0;
};

class CAP1166Component : public Component, public i2c::I2CDevice {
 public:
  void register_channel(CAP1166Channel *channel) { this->channels_.push_back(channel); }
  void register_channel(CAP1166LedChannel *channel);
  void set_touch_threshold(uint8_t touch_threshold) { this->touch_threshold_ = touch_threshold; };
  void set_allow_multiple_touches(bool allow_multiple_touches) {
    this->allow_multiple_touches_ = allow_multiple_touches ? 0x41 : 0x80;
  };
  void set_link_leds(bool link_leds) {
    this->link_leds_ = link_leds ? 0xFF : 0x00;
  };

  void set_reset_pin(GPIOPin *reset_pin) { this->reset_pin_ = reset_pin; }
  void setup() override;
  void dump_config() override;
  void loop() override;
  void turn_on(uint8_t channel);
  void turn_off(uint8_t channel);
  void configure_led_behavior(uint8_t channel, CAP1166LedBehavior behavior);

 protected:
  void finish_setup_();

  std::vector<CAP1166Channel *> channels_{};
  std::vector<CAP1166LedChannel *> led_channels_{};
  uint8_t led_channels_mask_{0x00};

  uint8_t touch_threshold_{0x20};
  uint8_t allow_multiple_touches_{0x80};
  uint8_t link_leds_{0xFF};

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
