# ESPHome External Components

This repository contains ESPHome external components for:
- [**CAP1166**](#cap1166-capacitive-touch-sensor): I2C/SPI capacitive touch sensor
- [**TCA9554**](#tca9554-io-expander): I2C 8-pin GPIO expander

These components are based on the official ESPHome components:
- cap1166 is based on [cap1188](https://esphome.io/components/binary_sensor/cap1188/)
- tca9554 is based on [tca9555](https://esphome.io/components/tca9555/)

---

## CAP1166 Capacitive Touch Sensor

The `cap1166` component allows you to use the Microchip CAP1166 capacitive touch sensor with ESPHome.
It supports up to 6 touch channels and integrated LED controls for each channel.

The `cap1166` sensor platform allows you to use Microchip CAP1166 ([datasheet](https://ww1.microchip.com/downloads/aemDocuments/documents/OTH/ProductDocuments/DataSheets/00001621B.pdf)) Capacitive Touch Sensor with ESPHome. The [I²C](https://esphome.io/components/i2c) bus is required to be set up in your configuration for this sensor to work.

The CAP1166 provides 6 independent capacitive touch channels and 6 integrated LED drivers, commonly used in touch interfaces.

> ℹ️ Note
> 
> SPI is not currently supported. I²C must be used at this time.

### Implementation Details

This started as a copy of cap1188 component but I expanded it to support LEDs. The original cap1188 will
support cap1166 with a simple change:

```c++
  if ((this->cap1166_product_id_ != 0x51) || (this->cap1166_manufacture_id_ != 0x5D))
```

I believe cap1166 and cap1188 are compatibile, but cap1188 component checks for product id and doesn't
allow cap1166. Product id 0x50 is cap1188 and product id 0x51 is cap1166.

### Example Configuration

I used this component for [my Touch pHat](https://esphome.atmyworkshop.online/devices/touch-phat/) by Pimoroni.
And I used that in my [bedroom climate controller project](https://esphome.atmyworkshop.online/projects/bedroom-climate-controller-touch/#main-configuration-file)

```yaml
external_components:
  - source: github://barbarachbc/esphomecomponents
    components: [ cap1166 ]
    refresh: 5min

i2c:
  sda: GPIO21
  scl: GPIO22

cap1166:
  - id: touch_phat
    address: 0x2C
    touch_threshold: 0x40
    allow_multiple_touches: true

---
  - platform: cap1166
    id: touch_key0
    channel: 0
    name: "Touch Key 0"
    on_press:
      then:
        - logger.log: "Key 0 pressed"
    on_release:
      then:
        - logger.log: "Key 0 released"

light:
  - id: my_light_forward
    platform: cap1166
    channel: 0
    led_behavior: PULSE2
```

### Configuration variables

The configuration is made up of two parts: The central component, and individual binary sensors and lights per channel.

These are the same as for CAP1188:

* **address** (_Optional_, int): The I²C address of the sensor. Defaults to `0x2C`.
* **id** (_Required_, [ID](https://esphome.io/guides/configuration-types#id)): Set the ID of this sensor.
* **reset_pin** (_Optional_, [Pin](https://esphome.io/guides/configuration-types#pin)): Set the pin that is used to reset the CAP1166 board on boot.
* **touch_threshold** (_Optional_, int): The touch threshold for all channels. This defines the sensitivity for touch detection.
    * `0x01` : Maximum sensitivity - Most sensitive to touch
    * `0x20` : Default sensitivity
    * `0x40` : Medium sensitivity (works through 3mm plastic)
    * `0x80` : Minimum sensitivity - Least sensitive to touch
* **allow_multiple_touches** (_Optional_, boolean): Whether to allow multitouch. Defaults to off.

New options:

* **brightness_configs** (_Optional_, list): Configure LED behavior and brightness for each channel. Each item can set `led_behavior` (DIRECT, PULSE1, PULSE2, BREATHE) and `max_brightness` (percentage).

The configuration is setting the brightness for the led behaviour across all LEDs. It cannot be configured
per LED.

```yaml
cap1166:
    - id: touch_phat
        address: 0x2C
        touch_threshold: 0x40
        allow_multiple_touches: false
        brightness_configs:
            - led_behavior: DIRECT
              max_brightness: 40%
            - led_behavior: PULSE1
              max_brightness: 40%
            - led_behavior: PULSE2
              max_brightness: 40%
            - led_behavior: BREATHE
              max_brightness: 40%
```

#### Binary Sensor

The `cap1166` binary sensor allows you to use your CAP1166 with ESPHome. First, setup a [Component/Hub](#configuration-variables) and then use this binary sensor platform to create individual binary sensors for each touch sensor.

Configuration variables:

* **cap1166_id** (_Optional_, [ID](https://esphome.io/guides/configuration-types#id)): The ID of the CAP1166 defined above. Useful for multiple CAP1166’s on the I²C bus.
* **channel** (**Required**, int): The channel number the CAP1166 touchkey is connected to (0-5).
* All other options from [Binary Sensor](https://esphome.io/components/binary_sensor#config-binary_sensor).

```yaml
binary_sensor:
    - platform: cap1166
        id: touch_A
        channel: 1
        on_click:
            then:
                - script.execute: a_button_click
    # ... more channels ...
```

#### Light

The `cap1166` light allows you to use your CAP1166 LED drivers with ESPHome. First, setup a [Component/Hub](#configuration-variables) and then use this light platform to create individual lights for each LED driver.

Configuration variables:

* **cap1166_id** (_Optional_, [ID](https://esphome.io/guides/configuration-types#id)): The ID of the CAP1166 defined above.
* **channel** (**Required**, int): The channel number (0-5).
* **led_behavior** (_optional_, string): LED mode: DIRECT, PULSE1, PULSE2 or BREATHE. Defaults to _DIRECT_.
* **linked** (_Optional_, boolean): Defaults to _false_.
* All other options from [Light](https://esphome.io/components/light/).

**NOTE**: At least one of **id** or **name** is required to be configured. If _name_ is configured
(or _internal_ is false), the light will appear in Home Assistant (if Home Assistant configured).
If _linked_ is true, the light can only be configured to be _internal_ because linked means that the
LED channel is controller by the corresponding touch channel and cannot be manually set.

```yaml
light:
    - platform: cap1166
        id: my_light_a
        channel: 4
        led_behavior: DIRECT
```

---

## TCA9554 I/O Expander

The `tca9554` component allows you to use TCA9554 I/O expanders ([datasheet](https://www.ti.com/lit/ds/symlink/tca9554a.pdf)) in ESPHome. It uses I²C Bus for communication.

Once configured, you can use any of the 8 pins (TCA9554) as pins for your projects. Within ESPHome they emulate a real internal GPIO pin and can therefore be used with many of ESPHome’s components such as the GPIO binary sensor or GPIO switch.

Any option accepting a [Pin Schema](https://esphome.io/guides/configuration-types#pin-schema) can theoretically be used, but some more complicated components that do communication through this I/O expander will not work.

> ℹ️ Note
> 
> It is possible to use these GPIOs as software SPI bus. See examples below.
>
> You can use [ESPHome pca9554](https://esphome.io/components/pca9554/) component directly because
> they are compatibile (you can use `pca9554` for TCA9555 too I'm sure because the devices are compatibile).
>
> ⚠️ Warning
>
> Using `pca9554` component I could not implement software SPI, that's why I kept my `tca9554` implementation

### Implementation Details

This is a copy of `tca9555` component but I removed 2nd byte of each operation. `tca9554` and `tca9555` are
not compatible because `tca9554` uses a single byte for configuration of the GPIOs, and `tca9555` uses
2 bytes so the register addresses are not the same. The whole change I did was to remove sending the 2nd
byte.

I used this component for [my button shim](https://esphome.atmyworkshop.online/devices/button-shim/) by Pimoroni. It is important to note that I did try `pca9554` component (which supports both `pca9554` and
`pca9555`) and this did work for the buttons on the board. But the board also has APA102 RGB LED which
uses SPI LED strip platform. This did not work with `pca9554` component, it looks like SPI did not work
for some reason.

### Example Configuration

```yaml
external_components:
  - source: github://barbarachbc/esphomecomponents
    components: [ tca9554 ]
    refresh: 5min

i2c:
  sda: GPIO21
  scl: GPIO22

tca9554:
  - id: btn_shim
    address: 0x3f

binary_sensor:
  - platform: gpio
    pin:
      tca9554: btn_shim
      number: 0
      inverted: true
    id: btn_a
    on_press:
      then:
        - light.addressable_set:
            id: expander_led
            range_from: 0
            range_to: 0
            red: 100%
            green: 22%
            blue: 12%
    on_release:
      then:
        - light.addressable_set:
            id: expander_led
            range_from: 0
            range_to: 0
            red: 0%
            green: 0%
            blue: 0%

spi:
  - id: expander_led_spi
    clk_pin:
      tca9554: btn_shim
      number: 6
    mosi_pin:
      tca9554: btn_shim
      number: 7
    interface: software

light:
  - platform: spi_led_strip
    num_leds: 1
    id: expander_led
    spi_id: expander_led_spi
    data_rate: 1kHz

```

#### Binary Sensor

You could use expander connected to a button like this:

```yaml
binary_sensor:
  - platform: gpio
    pin:
      tca9554: btn_shim
      number: 0
      inverted: true
    id: btn_a
    on_press:
      then:
      ....
```

#### Switch

Or you could use it as a switch for example:

```yaml
binary_sensor:
  - platform: gpio
    pin:
      tca9554: btn_shim
      number: 1
      inverted: false
      mode: OUTPUT
```

#### Software SPI

The button shim uses pins with numbers 6 & 7 to connect to APA102 LED. This is an example how to configure
SPI with pins that are not GPIOs (from other expanders for example). Note that `light` component is there
for the button shim's LED and it demonstrates that other components can use this software SPI bus.

```yaml
spi:
  - id: expander_led_spi
    clk_pin:
      tca9554: btn_shim
      number: 6
    mosi_pin:
      tca9554: btn_shim
      number: 7
    interface: software
```

### Configuration variables

These are the same as for `tca9555`:
* **id** (**Required**, [ID](https://esphome.io/guides/configuration-types#id)): Set the ID of this TCA9554 component.
* **address** (_Optional_, int): The I²C address of the expander. Defaults to `0x38` (because that's the
address of my Button Shim ☺️). Allowed addresses are: for TCA9554 (`0x20 - 0x27`) and for TCA9554A
(`0x38 - 0x3F`). These should be compatibile with PCA9554 and PCA9554A.

#### Pin configuration variables
* **tca9554** (**Required**, [ID](https://esphome.io/guides/configuration-types#id)): The id of the TCA9554 component of the pin.
* **number** (**Required**, int): The pin number (0-7).
* **inverted** (_Optional_, boolean): If all read and written values should be treated as inverted. Defaults to `false`.
* **mode** (_Optional_, string): A pin mode to set for the pin at. One of `INPUT` or `OUTPUT`.

---

## References
- [ESPHome cap1188](https://esphome.io/components/binary_sensor/cap1188/)
- [ESPHome pca9554](https://esphome.io/components/pca9554/)
- [ESPHome tca9555](https://esphome.io/components/tca9555/)

---

## Notes
- These components are external and may not be as stable as official ESPHome components.
- For issues or improvements, see the [barbarachbc/esphomecomponents](https://github.com/barbarachbc/esphomecomponents) repository.
