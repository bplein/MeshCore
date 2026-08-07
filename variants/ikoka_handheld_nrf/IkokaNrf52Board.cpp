#ifdef IKOKA_NRF52

#include <Arduino.h>
#include <Wire.h>

#include "IkokaNrf52Board.h"

#ifdef NRF52_POWER_MANAGEMENT
// Static configuration for power management
// Values set in variant.h defines
const PowerMgtConfig power_config = {
  .lpcomp_ain_channel = PWRMGT_LPCOMP_AIN,
  .lpcomp_refsel = PWRMGT_LPCOMP_REFSEL,
  .voltage_bootlock = PWRMGT_VOLTAGE_BOOTLOCK
};

void IkokaNrf52Board::initiateShutdown(uint8_t reason) {
  bool enable_lpcomp = (reason == SHUTDOWN_REASON_LOW_VOLTAGE ||
                        reason == SHUTDOWN_REASON_BOOT_PROTECT);

  pinMode(VBAT_ENABLE, OUTPUT);
  digitalWrite(VBAT_ENABLE, enable_lpcomp ? LOW : HIGH);

  if (enable_lpcomp) {
    configureVoltageWake(power_config.lpcomp_ain_channel, power_config.lpcomp_refsel);
  }

  enterSystemOff(reason);
}
#endif // NRF52_POWER_MANAGEMENT

void IkokaNrf52Board::begin() {
  NRF52BoardDCDC::begin();

  // ensure we have pull ups on the screen i2c, this isn't always available
  // in hardware and it should only be 20k ohms. Disable the pullups if we
  // are using the rotated lcd breakout board
  #if defined(DISPLAY_CLASS) && DISPLAY_ROTATION == 0
    pinMode(PIN_WIRE_SDA, INPUT_PULLUP);
    pinMode(PIN_WIRE_SCL, INPUT_PULLUP);
  #endif

  pinMode(PIN_VBAT, INPUT);
  pinMode(VBAT_ENABLE, OUTPUT);
  digitalWrite(VBAT_ENABLE, HIGH);

  // required button pullup is handled as part of button initilization
  // in target.cpp

#ifdef PIN_USER_BTN
  pinMode(PIN_USER_BTN, INPUT_PULLUP);
#endif

#if defined(PIN_WIRE_SDA) && defined(PIN_WIRE_SCL)
  Wire.setPins(PIN_WIRE_SDA, PIN_WIRE_SCL);
#endif

  Wire.begin();

#ifdef P_LORA_TX_LED
  pinMode(P_LORA_TX_LED, OUTPUT);
  digitalWrite(P_LORA_TX_LED, !LED_STATE_ON);   // boot idle = OFF (HIGH for active-LOW)
#endif

#ifdef NRF52_POWER_MANAGEMENT
  // Boot voltage protection check (may not return if voltage too low)
  checkBootVoltage(&power_config);
#endif

  delay(10);  // Give sx1262 some time to power up
}

uint16_t IkokaNrf52Board::getBattMilliVolts() {
  // https://wiki.seeedstudio.com/XIAO_BLE#q3-what-are-the-considerations-when-using-xiao-nrf52840-sense-for-battery-charging
  // VBAT_ENABLE must be LOW to read battery voltage
  // ikoka's begin() does NOT configure the ADC (unlike xiao), so set it here
  digitalWrite(VBAT_ENABLE, LOW);
  int adcvalue = 0;
  analogReadResolution(12);
  analogReference(AR_INTERNAL_3_0);
  delay(10);
  adcvalue = analogRead(PIN_VBAT);
  return (adcvalue * ADC_MULTIPLIER * AREF_VOLTAGE) / 4.096;
}

#endif
