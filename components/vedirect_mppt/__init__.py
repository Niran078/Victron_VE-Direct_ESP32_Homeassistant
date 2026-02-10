import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import uart, sensor, text_sensor, binary_sensor
from esphome.const import CONF_ID
DEPENDENCIES = ["uart", "sensor", "text_sensor", "binary_sensor"]
AUTO_LOAD = ["sensor", "text_sensor", "binary_sensor"]


CONF_UART_ID = "uart_id"

# Sensor keys
CONF_BATTERY_VOLTAGE = "battery_voltage"
CONF_PANEL_VOLTAGE = "panel_voltage"
CONF_PANEL_POWER = "panel_power"
CONF_BATTERY_CURRENT = "battery_current"
CONF_LOAD_CURRENT = "load_current"
CONF_LOAD_OUTPUT_ON = "load_output_on"
CONF_OFF_REASON_RAW = "off_reason_raw"
CONF_OFF_REASON_TEXT = "off_reason_text"
CONF_STATE_CODE = "state_code"
CONF_STATE_TEXT = "state_text"
CONF_ERROR_CODE = "error_code"
CONF_MAX_POWER_TODAY = "max_power_today"
CONF_YIELD_TODAY = "yield_today"
CONF_YIELD_TOTAL = "yield_total"
CONF_MPPT_MODE_CODE = "mppt_mode_code"
CONF_MPPT_MODE_TEXT = "mppt_mode_text"

# Extra diagnostics / derived
CONF_DATA_VALID = "data_valid"
CONF_FRAME_AGE = "frame_age"
CONF_FRAMES_OK = "frames_ok"
CONF_FRAMES_BAD = "frames_bad"
CONF_BATTERY_POWER = "battery_power"
CONF_LOAD_POWER = "load_power"
CONF_PV_EFF = "pv_efficiency"
CONF_LOAD_OUTPUT_TEXT = "load_output_text"

ved_ns = cg.esphome_ns.namespace("vedirect_mppt")
VEDirectMPPT = ved_ns.class_("VEDirectMPPT", cg.Component, uart.UARTDevice)

CONFIG_SCHEMA = (
    cv.Schema(
        {
            cv.GenerateID(): cv.declare_id(VEDirectMPPT),
            cv.Required(CONF_UART_ID): cv.use_id(uart.UARTComponent),

            cv.Optional(CONF_BATTERY_VOLTAGE): sensor.sensor_schema(),
            cv.Optional(CONF_PANEL_VOLTAGE): sensor.sensor_schema(),
            cv.Optional(CONF_PANEL_POWER): sensor.sensor_schema(),
            cv.Optional(CONF_BATTERY_CURRENT): sensor.sensor_schema(),
            cv.Optional(CONF_LOAD_CURRENT): sensor.sensor_schema(),

            cv.Optional(CONF_LOAD_OUTPUT_ON): binary_sensor.binary_sensor_schema(),
            cv.Optional(CONF_LOAD_OUTPUT_TEXT): text_sensor.text_sensor_schema(),

            cv.Optional(CONF_OFF_REASON_RAW): sensor.sensor_schema(),
            cv.Optional(CONF_OFF_REASON_TEXT): text_sensor.text_sensor_schema(),

            cv.Optional(CONF_STATE_CODE): sensor.sensor_schema(),
            cv.Optional(CONF_STATE_TEXT): text_sensor.text_sensor_schema(),

            cv.Optional(CONF_ERROR_CODE): sensor.sensor_schema(),

            cv.Optional(CONF_MAX_POWER_TODAY): sensor.sensor_schema(),
            cv.Optional(CONF_YIELD_TODAY): sensor.sensor_schema(),
            cv.Optional(CONF_YIELD_TOTAL): sensor.sensor_schema(),

            cv.Optional(CONF_MPPT_MODE_CODE): sensor.sensor_schema(),
            cv.Optional(CONF_MPPT_MODE_TEXT): text_sensor.text_sensor_schema(),

            # Extras
            cv.Optional(CONF_DATA_VALID): binary_sensor.binary_sensor_schema(),
            cv.Optional(CONF_FRAME_AGE): sensor.sensor_schema(),
            cv.Optional(CONF_FRAMES_OK): sensor.sensor_schema(),
            cv.Optional(CONF_FRAMES_BAD): sensor.sensor_schema(),
            cv.Optional(CONF_BATTERY_POWER): sensor.sensor_schema(),
            cv.Optional(CONF_LOAD_POWER): sensor.sensor_schema(),
            cv.Optional(CONF_PV_EFF): sensor.sensor_schema(),
        }
    )
    .extend(cv.COMPONENT_SCHEMA)
)

async def to_code(config):
    parent = await cg.get_variable(config[CONF_UART_ID])
    var = cg.new_Pvariable(config[CONF_ID], parent)
    await cg.register_component(var, config)
    await uart.register_uart_device(var, config)

    # --- Raw sensors ---
    if CONF_BATTERY_VOLTAGE in config:
        s = await sensor.new_sensor(config[CONF_BATTERY_VOLTAGE])
        cg.add(var.set_battery_voltage(s))

    if CONF_PANEL_VOLTAGE in config:
        s = await sensor.new_sensor(config[CONF_PANEL_VOLTAGE])
        cg.add(var.set_panel_voltage(s))

    if CONF_PANEL_POWER in config:
        s = await sensor.new_sensor(config[CONF_PANEL_POWER])
        cg.add(var.set_panel_power(s))

    if CONF_BATTERY_CURRENT in config:
        s = await sensor.new_sensor(config[CONF_BATTERY_CURRENT])
        cg.add(var.set_battery_current(s))

    if CONF_LOAD_CURRENT in config:
        s = await sensor.new_sensor(config[CONF_LOAD_CURRENT])
        cg.add(var.set_load_current(s))

    # --- Load output ---
    if CONF_LOAD_OUTPUT_ON in config:
        b = await binary_sensor.new_binary_sensor(config[CONF_LOAD_OUTPUT_ON])
        cg.add(var.set_load_output_on(b))

    if CONF_LOAD_OUTPUT_TEXT in config:
        t = await text_sensor.new_text_sensor(config[CONF_LOAD_OUTPUT_TEXT])
        cg.add(var.set_load_output_text(t))

    # --- Off reason ---
    if CONF_OFF_REASON_RAW in config:
        s = await sensor.new_sensor(config[CONF_OFF_REASON_RAW])
        cg.add(var.set_off_reason_raw(s))

    if CONF_OFF_REASON_TEXT in config:
        t = await text_sensor.new_text_sensor(config[CONF_OFF_REASON_TEXT])
        cg.add(var.set_off_reason_text(t))

    # --- State / error ---
    if CONF_STATE_CODE in config:
        s = await sensor.new_sensor(config[CONF_STATE_CODE])
        cg.add(var.set_state_code(s))

    if CONF_STATE_TEXT in config:
        t = await text_sensor.new_text_sensor(config[CONF_STATE_TEXT])
        cg.add(var.set_state_text(t))

    if CONF_ERROR_CODE in config:
        s = await sensor.new_sensor(config[CONF_ERROR_CODE])
        cg.add(var.set_error_code(s))

    # --- Energy stats ---
    if CONF_MAX_POWER_TODAY in config:
        s = await sensor.new_sensor(config[CONF_MAX_POWER_TODAY])
        cg.add(var.set_max_power_today(s))

    if CONF_YIELD_TODAY in config:
        s = await sensor.new_sensor(config[CONF_YIELD_TODAY])
        cg.add(var.set_yield_today(s))

    if CONF_YIELD_TOTAL in config:
        s = await sensor.new_sensor(config[CONF_YIELD_TOTAL])
        cg.add(var.set_yield_total(s))

    # --- MPPT mode ---
    if CONF_MPPT_MODE_CODE in config:
        s = await sensor.new_sensor(config[CONF_MPPT_MODE_CODE])
        cg.add(var.set_mppt_mode_code(s))

    if CONF_MPPT_MODE_TEXT in config:
        t = await text_sensor.new_text_sensor(config[CONF_MPPT_MODE_TEXT])
        cg.add(var.set_mppt_mode_text(t))

    # --- Extras / diagnostics ---
    if CONF_DATA_VALID in config:
        b = await binary_sensor.new_binary_sensor(config[CONF_DATA_VALID])
        cg.add(var.set_data_valid(b))

    if CONF_FRAME_AGE in config:
        s = await sensor.new_sensor(config[CONF_FRAME_AGE])
        cg.add(var.set_frame_age(s))

    if CONF_FRAMES_OK in config:
        s = await sensor.new_sensor(config[CONF_FRAMES_OK])
        cg.add(var.set_frames_ok(s))

    if CONF_FRAMES_BAD in config:
        s = await sensor.new_sensor(config[CONF_FRAMES_BAD])
        cg.add(var.set_frames_bad(s))

    if CONF_BATTERY_POWER in config:
        s = await sensor.new_sensor(config[CONF_BATTERY_POWER])
        cg.add(var.set_battery_power(s))

    if CONF_LOAD_POWER in config:
        s = await sensor.new_sensor(config[CONF_LOAD_POWER])
        cg.add(var.set_load_power(s))

    if CONF_PV_EFF in config:
        s = await sensor.new_sensor(config[CONF_PV_EFF])
        cg.add(var.set_pv_efficiency(s))
