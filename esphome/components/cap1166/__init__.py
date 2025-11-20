from esphome import pins
import esphome.codegen as cg
from esphome.components import i2c
import esphome.config_validation as cv
from esphome.const import CONF_ID, CONF_RESET_PIN

CONF_TOUCH_THRESHOLD = "touch_threshold"
CONF_ALLOW_MULTIPLE_TOUCHES = "allow_multiple_touches"
CONF_LINK_LEDS = "link_leds"
CONF_BRIGHTNESS_CONFIGS = "brightness_configs"
CONF_LED_BEHAVIOR = "led_behavior"
CONF_MAX_BRIGHTNESS = "max_brightness"
CONF_MIN_BRIGHTNESS = "min_brightness"

DEPENDENCIES = ["i2c"]
AUTO_LOAD = ["binary_sensor", "output"]
CODEOWNERS = ["@barbarachbc"]

cap1166_ns = cg.esphome_ns.namespace("cap1166")
CONF_CAP1166_ID = "cap1166_id"
CAP1166Component = cap1166_ns.class_("CAP1166Component", cg.Component, i2c.I2CDevice)

# LED Behavior enum for brightness configuration
CAP1166LedBehavior = cap1166_ns.enum("CAP1166LedBehavior")
LED_BEHAVIORS = {
    "DIRECT": CAP1166LedBehavior.LED_BEHAVIOR_DIRECT,
    "PULSE1": CAP1166LedBehavior.LED_BEHAVIOR_PULSE1,
    "PULSE2": CAP1166LedBehavior.LED_BEHAVIOR_PULSE2,
    "BREATHE": CAP1166LedBehavior.LED_BEHAVIOR_BREATHE,
}

# Schema for individual brightness configuration per behavior
BRIGHTNESS_CONFIG_SCHEMA = cv.Schema({
    cv.Required(CONF_LED_BEHAVIOR): cv.enum(LED_BEHAVIORS, upper=True),
    cv.Optional(CONF_MAX_BRIGHTNESS, default=1.0): cv.percentage,  # 0-100%
    cv.Optional(CONF_MIN_BRIGHTNESS, default=0.0): cv.percentage,    # 0-100%
})

def validate_brightness_config(config):
    """Validate that max_brightness >= min_brightness for each behavior config."""    
    for brightness_config in config.get(CONF_BRIGHTNESS_CONFIGS, []):
        max_brightness = brightness_config[CONF_MAX_BRIGHTNESS]
        min_brightness = brightness_config[CONF_MIN_BRIGHTNESS]
        
        if max_brightness <= min_brightness:
            raise cv.Invalid(
                f"max_brightness ({max_brightness*100:.1f}%) must be > min_brightness ({min_brightness*100:.1f}%) "
                f"for behavior {brightness_config[CONF_LED_BEHAVIOR]}"
            )
    
    return config


MULTI_CONF = True
CONFIG_SCHEMA = cv.All(
    cv.Schema(
        {
            cv.GenerateID(): cv.declare_id(CAP1166Component),
            cv.Optional(CONF_RESET_PIN): pins.gpio_output_pin_schema,
            cv.Optional(CONF_TOUCH_THRESHOLD, default=0x20): cv.int_range(
                min=0x01, max=0x80
            ),
            cv.Optional(CONF_ALLOW_MULTIPLE_TOUCHES, default=False): cv.boolean,
            cv.Optional(CONF_LINK_LEDS, default=True): cv.boolean,
            cv.Optional(CONF_BRIGHTNESS_CONFIGS, default=[]): cv.ensure_list(
                BRIGHTNESS_CONFIG_SCHEMA
            ),
        }
    )
    .extend(cv.COMPONENT_SCHEMA)
    .extend(i2c.i2c_device_schema(0x29)),
    validate_brightness_config,
)


async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    cg.add(var.set_touch_threshold(config[CONF_TOUCH_THRESHOLD]))
    cg.add(var.set_allow_multiple_touches(config[CONF_ALLOW_MULTIPLE_TOUCHES]))
    cg.add(var.set_link_leds(config[CONF_LINK_LEDS]))

    if reset_pin_config := config.get(CONF_RESET_PIN):
        pin = await cg.gpio_pin_expression(reset_pin_config)
        cg.add(var.set_reset_pin(pin))

    # Configure brightness settings per behavior
    # Convert percentages (0.0-1.0) to actual percentages (0-100)
    for brightness_config in config.get(CONF_BRIGHTNESS_CONFIGS, []):
        behavior = brightness_config[CONF_LED_BEHAVIOR]
        max_brightness = int(brightness_config[CONF_MAX_BRIGHTNESS] * 100)  # Convert to 0-100
        min_brightness = int(brightness_config[CONF_MIN_BRIGHTNESS] * 100)  # Convert to 0-100
        
        cg.add(var.set_behavior_brightness(behavior, max_brightness, min_brightness))

    await cg.register_component(var, config)
    await i2c.register_i2c_device(var, config)
