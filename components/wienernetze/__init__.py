# ruff: noqa: ANN001, ANN201

import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import uart
from esphome.const import CONF_ID, CONF_RAW_DATA_ID

DEPENDENCIES = ["uart"]
CODEOWNERS = ["@bernikr"]
AUTO_LOAD = ["text_sensor", "sensor"]
MULTI_CONF = True

CONF_WIENERNETZE_ID = "wienernetze_id"
CONG_WIENERNETZE_KEY = "key"

wienernetze_ns = cg.esphome_ns.namespace("wienernetze")
wienernetze_component = wienernetze_ns.class_("WienerNetze", cg.Component)

WIENERNETZE_COMPONENT_SCHEMA = cv.Schema(
    {
        cv.GenerateID(CONF_WIENERNETZE_ID): cv.use_id(wienernetze_component),
    },
)


def validate_key(value):
    value = cv.string(value).replace(" ", "").upper()
    if len(value) != 32 or any(c not in "0123456789ABCDEF" for c in value):  # noqa: PLR2004
        msg = "Key must be 16 bytes in Hex"
        raise cv.Invalid(msg)
    return [int(value[i * 2 : i * 2 + 2], 16) for i in range(16)]


CONFIG_SCHEMA = cv.All(
    cv.Schema(
        {
            cv.GenerateID(): cv.declare_id(wienernetze_component),
            cv.Required(CONG_WIENERNETZE_KEY): validate_key,
            cv.GenerateID(CONF_RAW_DATA_ID): cv.declare_id(cg.uint8),
        },
    ).extend(uart.UART_DEVICE_SCHEMA),
)


def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    arr = cg.progmem_array(config[CONF_RAW_DATA_ID], config[CONG_WIENERNETZE_KEY])
    cg.add(var.set_key(arr))
    yield cg.register_component(var, config)
    yield uart.register_uart_device(var, config)
    cg.add_library("frankboesing/FastCRC", "1.41")
    cg.add_library("rweather/Crypto", "0.4.0")
