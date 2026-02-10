#pragma once
#include <cmath>
#include <string>
#include "esphome/core/component.h"
#include "esphome/components/uart/uart.h"
#include "esphome/components/sensor/sensor.h"
#include "esphome/components/text_sensor/text_sensor.h"
#include "esphome/components/binary_sensor/binary_sensor.h"

namespace esphome {
namespace vedirect_mppt {

static const char *cs_to_text(int cs) {
  switch (cs) {
    case 0: return "Off";
    case 1: return "Low power";
    case 2: return "Fault";
    case 3: return "Bulk";
    case 4: return "Absorption";
    case 5: return "Float";
    case 6: return "Storage";
    case 7: return "Equalize (manual)";
    case 11: return "Power supply";
    case 245: return "Starting-up";
    case 246: return "Repeated absorption";
    case 247: return "Auto equalize / Recondition";
    case 248: return "BatterySafe";
    case 252: return "External control";
    default: return "Unknown";
  }
}

static const char *mppt_to_text(int m) {
  switch (m) {
    case 0: return "Off";
    case 1: return "Voltage/current limited";
    case 2: return "MPP Tracker active";
    default: return "Unknown";
  }
}

static uint32_t parse_u32_flexible_(const std::string &s) {
  const char *c = s.c_str();
  if (s.size() > 2 && s[0] == '0' && (s[1] == 'x' || s[1] == 'X')) return (uint32_t) strtoul(c, nullptr, 16);
  return (uint32_t) strtoul(c, nullptr, 10);
}

static std::string off_reason_to_text_(uint32_t or_mask) {
  if (or_mask == 0) return "None";
  struct Bit { uint32_t bit; const char *name; };
  static const Bit bits[] = {
    {0x00000001, "No input power"},
    {0x00000002, "Switched off (power switch)"},
    {0x00000004, "Switched off (device mode)"},
    {0x00000008, "Remote input"},
    {0x00000010, "Protection active"},
    {0x00000020, "Paygo"},
    {0x00000040, "BMS"},
    {0x00000080, "Engine shutdown detection"},
    {0x00000100, "Analysing input voltage"},
  };
  std::string out;
  for (auto &b : bits) {
    if (or_mask & b.bit) {
      if (!out.empty()) out += ", ";
      out += b.name;
    }
  }
  if (out.empty()) out = "Unknown";
  return out;
}

class VEDirectMPPT : public Component, public uart::UARTDevice {
 public:
  explicit VEDirectMPPT(uart::UARTComponent *parent) : uart::UARTDevice(parent) {}

  // Setters from codegen
  void set_battery_voltage(sensor::Sensor *s) { battery_v_ = s; }
  void set_panel_voltage(sensor::Sensor *s) { panel_v_ = s; }
  void set_panel_power(sensor::Sensor *s) { panel_w_ = s; }
  void set_battery_current(sensor::Sensor *s) { battery_a_ = s; }
  void set_load_current(sensor::Sensor *s) { load_a_ = s; }

  void set_load_output_on(binary_sensor::BinarySensor *b) { load_on_ = b; }
  void set_load_output_text(text_sensor::TextSensor *t) { load_text_ = t; }

  void set_off_reason_raw(sensor::Sensor *s) { off_reason_raw_ = s; }
  void set_off_reason_text(text_sensor::TextSensor *t) { off_reason_text_ = t; }

  void set_state_code(sensor::Sensor *s) { cs_code_ = s; }
  void set_state_text(text_sensor::TextSensor *t) { cs_text_ = t; }

  void set_error_code(sensor::Sensor *s) { err_code_ = s; }

  void set_max_power_today(sensor::Sensor *s) { h21_w_ = s; }
  void set_yield_today(sensor::Sensor *s) { h20_kwh_ = s; }
  void set_yield_total(sensor::Sensor *s) { h19_kwh_ = s; }

  void set_mppt_mode_code(sensor::Sensor *s) { mppt_code_ = s; }
  void set_mppt_mode_text(text_sensor::TextSensor *t) { mppt_text_ = t; }

  void set_data_valid(binary_sensor::BinarySensor *b) { data_valid_ = b; }
  void set_frame_age(sensor::Sensor *s) { frame_age_s_ = s; }
  void set_frames_ok(sensor::Sensor *s) { frames_ok_ = s; }
  void set_frames_bad(sensor::Sensor *s) { frames_bad_ = s; }
  void set_battery_power(sensor::Sensor *s) { batt_power_w_ = s; }
  void set_load_power(sensor::Sensor *s) { load_power_w_ = s; }
  void set_pv_efficiency(sensor::Sensor *s) { pv_eff_pct_ = s; }

  void loop() override {
    const uint32_t now = millis();
    if (now - last_diag_ms_ >= 1000) {
      last_diag_ms_ = now;
      const bool valid = (last_valid_ms_ != 0) && (now - last_valid_ms_ <= valid_window_ms_);
      if (data_valid_ != nullptr) data_valid_->publish_state(valid);
      if (frame_age_s_ != nullptr) {
        if (last_valid_ms_ == 0) frame_age_s_->publish_state(NAN);
        else frame_age_s_->publish_state((now - last_valid_ms_) / 1000.0f);
      }
      if (frames_ok_ != nullptr) frames_ok_->publish_state((float) ok_count_);
      if (frames_bad_ != nullptr) frames_bad_->publish_state((float) bad_count_);
    }

    while (available()) {
      uint8_t b;
      read_byte(&b);

      checksum_sum_ = (uint8_t) (checksum_sum_ + b);

      if (line_len_ < sizeof(line_buf_) - 1) line_buf_[line_len_++] = (char) b;
      else line_len_ = 0;

      if (b == '\n') {
        process_line_();
        line_len_ = 0;
      }
    }
  }

 private:
  struct Temp {
    bool has_v=false, has_vpv=false, has_ppv=false, has_i=false, has_il=false;
    bool has_load=false, has_or=false, has_cs=false, has_err=false, has_h21=false, has_h20=false, has_h19=false, has_mppt=false;

    float v=NAN, vpv=NAN, ppv=NAN, i=NAN, il=NAN;
    bool load_on=false;
    uint32_t or_mask=0;
    int cs=0, err=0, mppt=0;
    float h21=NAN, h20_kwh=NAN, h19_kwh=NAN;
  } temp_;

  uint8_t checksum_sum_ = 0;
  char line_buf_[96];
  size_t line_len_ = 0;

  uint32_t last_valid_ms_ = 0;
  uint32_t last_diag_ms_ = 0;
  const uint32_t valid_window_ms_ = 5000;
  uint32_t ok_count_ = 0;
  uint32_t bad_count_ = 0;

  // Outputs (nullable)
  sensor::Sensor *battery_v_{nullptr};
  sensor::Sensor *panel_v_{nullptr};
  sensor::Sensor *panel_w_{nullptr};
  sensor::Sensor *battery_a_{nullptr};
  sensor::Sensor *load_a_{nullptr};

  binary_sensor::BinarySensor *load_on_{nullptr};
  text_sensor::TextSensor *load_text_{nullptr};

  sensor::Sensor *off_reason_raw_{nullptr};
  text_sensor::TextSensor *off_reason_text_{nullptr};

  sensor::Sensor *cs_code_{nullptr};
  text_sensor::TextSensor *cs_text_{nullptr};

  sensor::Sensor *err_code_{nullptr};

  sensor::Sensor *h21_w_{nullptr};
  sensor::Sensor *h20_kwh_{nullptr};
  sensor::Sensor *h19_kwh_{nullptr};

  sensor::Sensor *mppt_code_{nullptr};
  text_sensor::TextSensor *mppt_text_{nullptr};

  binary_sensor::BinarySensor *data_valid_{nullptr};
  sensor::Sensor *frame_age_s_{nullptr};
  sensor::Sensor *frames_ok_{nullptr};
  sensor::Sensor *frames_bad_{nullptr};

  sensor::Sensor *batt_power_w_{nullptr};
  sensor::Sensor *load_power_w_{nullptr};
  sensor::Sensor *pv_eff_pct_{nullptr};

  static std::string strip_crlf_(const std::string &s) {
    size_t a = 0;
    while (a < s.size() && (s[a] == '\r' || s[a] == '\n')) a++;
    size_t b = s.size();
    while (b > a && (s[b-1] == '\r' || s[b-1] == '\n')) b--;
    return s.substr(a, b - a);
  }

  void clear_temp_() { temp_ = Temp(); }

  void process_line_() {
    std::string raw(line_buf_, line_buf_ + line_len_);
    std::string s = strip_crlf_(raw);
    if (s.empty()) return;

    size_t tab = s.find('\t');
    if (tab == std::string::npos) return;

    std::string label = s.substr(0, tab);
    std::string value = s.substr(tab + 1);

    if (label == "Checksum") {
      if (checksum_sum_ == 0) {
        ok_count_++;
        last_valid_ms_ = millis();
        publish_temp_();
      } else {
        bad_count_++;
      }
      checksum_sum_ = 0;
      clear_temp_();
      return;
    }

    if (label == "V") {                temp_.v = atof(value.c_str()) / 1000.0f; temp_.has_v = true; }
    else if (label == "VPV") {         temp_.vpv = atof(value.c_str()) / 1000.0f; temp_.has_vpv = true; }
    else if (label == "PPV") {         temp_.ppv = atof(value.c_str()); temp_.has_ppv = true; }
    else if (label == "I") {           temp_.i = atof(value.c_str()) / 1000.0f; temp_.has_i = true; }
    else if (label == "IL") {          temp_.il = atof(value.c_str()) / 1000.0f; temp_.has_il = true; }
    else if (label == "LOAD") {
      std::string up = value;
      for (auto &c : up) c = (char) toupper((unsigned char) c);
      temp_.load_on = (up == "ON");
      temp_.has_load = true;
    }
    else if (label == "OR") {          temp_.or_mask = parse_u32_flexible_(value); temp_.has_or = true; }
    else if (label == "CS") {          temp_.cs = atoi(value.c_str()); temp_.has_cs = true; }
    else if (label == "ERR") {         temp_.err = atoi(value.c_str()); temp_.has_err = true; }
    else if (label == "H21") {         temp_.h21 = atof(value.c_str()); temp_.has_h21 = true; }
    else if (label == "H20") {         temp_.h20_kwh = atof(value.c_str()) / 100.0f; temp_.has_h20 = true; }
    else if (label == "H19") {         temp_.h19_kwh = atof(value.c_str()) / 100.0f; temp_.has_h19 = true; }
    else if (label == "MPPT") {        temp_.mppt = atoi(value.c_str()); temp_.has_mppt = true; }
  }

  void publish_temp_() {
    if (temp_.has_v && battery_v_) battery_v_->publish_state(temp_.v);
    if (temp_.has_vpv && panel_v_) panel_v_->publish_state(temp_.vpv);
    if (temp_.has_ppv && panel_w_) panel_w_->publish_state(temp_.ppv);
    if (temp_.has_i && battery_a_) battery_a_->publish_state(temp_.i);
    if (temp_.has_il && load_a_) load_a_->publish_state(temp_.il);

    if (temp_.has_h21 && h21_w_) h21_w_->publish_state(temp_.h21);
    if (temp_.has_h20 && h20_kwh_) h20_kwh_->publish_state(temp_.h20_kwh);
    if (temp_.has_h19 && h19_kwh_) h19_kwh_->publish_state(temp_.h19_kwh);

    if (temp_.has_err && err_code_) err_code_->publish_state((float) temp_.err);

    if (temp_.has_cs) {
      if (cs_code_) cs_code_->publish_state((float) temp_.cs);
      if (cs_text_) cs_text_->publish_state(cs_to_text(temp_.cs));
    }

    if (temp_.has_mppt) {
      if (mppt_code_) mppt_code_->publish_state((float) temp_.mppt);
      if (mppt_text_) mppt_text_->publish_state(mppt_to_text(temp_.mppt));
    }

    if (temp_.has_or) {
      if (off_reason_raw_) off_reason_raw_->publish_state((float) temp_.or_mask);
      if (off_reason_text_) off_reason_text_->publish_state(off_reason_to_text_(temp_.or_mask));
    }

    if (temp_.has_load) {
      if (load_on_) load_on_->publish_state(temp_.load_on);
      if (load_text_) load_text_->publish_state(temp_.load_on ? "ON" : "OFF");
    }

    // Derived
    if (temp_.has_v && temp_.has_i && batt_power_w_) {
      const float bp = temp_.v * temp_.i;
      batt_power_w_->publish_state(bp);
      if (temp_.has_ppv && pv_eff_pct_ && temp_.ppv > 1.0f) pv_eff_pct_->publish_state((bp / temp_.ppv) * 100.0f);
    }
    if (temp_.has_v && temp_.has_il && load_power_w_) {
      load_power_w_->publish_state(temp_.v * temp_.il);
    }
  }
};

}  // namespace vedirect_mppt
}  // namespace esphome
