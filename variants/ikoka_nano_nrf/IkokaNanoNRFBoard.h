#pragma once

#include <MeshCore.h>
#include <Arduino.h>
#include <helpers/NRF52Board.h>

#ifdef XIAO_NRF52

class IkokaNanoNRFBoard : public NRF52BoardDCDC {
protected:
#if NRF52_POWER_MANAGEMENT
  void initiateShutdown(uint8_t reason) override;
#endif

public:
  IkokaNanoNRFBoard() : NRF52Board("XIAO_NRF52_OTA") {}
  void begin();

#if defined(P_LORA_TX_LED)
  void onBeforeTransmit() override {
    digitalWrite(P_LORA_TX_LED, LOW);   // turn TX LED on
    #if defined(LED_BLUE)
      // turn off that annoying blue LED before transmitting
      digitalWrite(LED_BLUE, HIGH);
    #endif
  }
  void onAfterTransmit() override {
    digitalWrite(P_LORA_TX_LED, HIGH);   // turn TX LED off
    #if defined(LED_BLUE)
       // do it after transmitting too, just in case
      digitalWrite(LED_BLUE, HIGH);
    #endif
  }
#endif

  uint16_t getBattMilliVolts() override;

  const char *getManufacturerName() const override {
    return MANUFACTURER_STRING;
  }
};

#endif
