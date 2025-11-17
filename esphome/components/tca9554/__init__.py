from esphome import pins
import esphome.codegen as cg
from esphome.components import i2c
import esphome.config_validation as cv
from esphome.const import (
    CONF_ID,
    CONF_INPUT,
    CONF_INVERTED,
    CONF_MODE,
    CONF_NUMBER,
    CONF_OUTPUT,
    CONF_ADDRESS
)

CODEOWNERS = ["@barbarachbc"]

AUTO_LOAD = ["gpio_expander"]
DEPENDENCIES = ["i2c"]
MULTI_CONF = True

#the only valid addresses for TCA9554 and TCA9554A
TCA9554_VALID_ADDRESSES = list(range(0x38, 0x40)) + list(range(0x20, 0x28))

tca9554_ns = cg.esphome_ns.namespace("tca9554")

TCA9554Component = tca9554_ns.class_("TCA9554Component", cg.Component, i2c.I2CDevice)
TCA9554GPIOPin = tca9554_ns.class_("TCA9554GPIOPin", cg.GPIOPin)

def check_keys(obj):
    if obj[CONF_ADDRESS] not in TCA9554_VALID_ADDRESSES:
        msg = "Only the following addresses are valid: 0x38-0x3F\r"
        msg += "TCA9554:  0x20 - 0x27\r"
        msg += "TCA9554A: 0x38 - 0x3F"
        raise cv.Invalid(msg)
    return obj

CONF_TCA9554 = "tca9554"
CONFIG_SCHEMA = cv.All(
    cv.Schema(
        {
            cv.Required(CONF_ID): cv.declare_id(TCA9554Component),
        }
    )
    .extend(cv.COMPONENT_SCHEMA)
    .extend(i2c.i2c_device_schema(0x38)),
    check_keys,
)

async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    await i2c.register_i2c_device(var, config)


def validate_mode(value):
    if not (value[CONF_INPUT] or value[CONF_OUTPUT]):
        raise cv.Invalid("Mode must be either input or output")
    if value[CONF_INPUT] and value[CONF_OUTPUT]:
        raise cv.Invalid("Mode must be either input or output")
    return value


TCA9554_PIN_SCHEMA = pins.gpio_base_schema(
    TCA9554GPIOPin,
    cv.int_range(min=0, max=7),
    modes=[CONF_INPUT, CONF_OUTPUT],
    mode_validator=validate_mode,
    invertible=True,
).extend(
    {
        cv.Required(CONF_TCA9554): cv.use_id(TCA9554Component),
    }
)


@pins.PIN_SCHEMA_REGISTRY.register(CONF_TCA9554, TCA9554_PIN_SCHEMA)
async def tca9554_pin_to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_parented(var, config[CONF_TCA9554])

    cg.add(var.set_pin(config[CONF_NUMBER]))
    cg.add(var.set_inverted(config[CONF_INVERTED]))
    cg.add(var.set_flags(pins.gpio_flags_expr(config[CONF_MODE])))
    return var
