#pragma once

#include "ESP32Board.h"
#include <driver/rtc_io.h>
#include <Wire.h>
#include <Arduino.h>
//#include "XPowersLib.h"

// Defined using AXP2102
//#define XPOWERS_CHIP_AXP2101

// LoRa radio module pins for TBeam S3 Supreme
#define  P_LORA_DIO_1   1   //SX1262 IRQ pin
#define  P_LORA_NSS     10  //SX1262 SS pin
#define  P_LORA_RESET   5   //SX1262 Rest pin
#define  P_LORA_BUSY    4   //SX1262 Busy pin
#define  P_LORA_SCLK    12  //SX1262 SCLK pin
#define  P_LORA_MISO    13  //SX1262 MISO pin
#define  P_LORA_MOSI    11  //SX1262 MOSI pin

#define PIN_BOARD_SDA_1 42  //SDA for PMU and PFC8563 (RTC)
#define PIN_BOARD_SCL_1 41  //SCL for PMU and PFC8563 (RTC)

#define PIN_USER_BTN 0

#define P_BOARD_SPI_MOSI 35  //SPI for SD Card and QMI8653 (IMU)
#define P_BOARD_SPI_MISO 37  //SPI for SD Card and QMI8653 (IMU)
#define P_BOARD_SPI_SCK  36  //SPI for SD Card and QMI8653 (IMU)
#define P_BPARD_SPI_CS   47  //SPI for SD Card and QMI8653 (IMU)
#define P_BOARD_IMU_CS   34  //Pin for QMI8653 (IMU) CS

#define P_BOARD_IMU_INT  33  //IMU Int pin
#define P_BOARD_RTC_INT  14  //RTC Int pin

#define P_GPS_RX    9   //GPS RX pin
#define P_GPS_TX    8   //GPS TX pin
#define P_GPS_WAKE  7   //GPS Wakeup pin
#define P_GPS_1PPS  6   //GPS 1PPS pin


class TBeamS3SupremeBoard : public ESP32Board {
  //XPowersAXP2101 PMU;

public:
  void begin() {
    ESP32Board::begin();
    
    //Manually set voltage rails
    //PMU.setProtectedChannel(XPOWERS_DCDC3); //Set protected DCDC for esp32
    //PMU.setALDO2Voltage(3300);  //Set LDO for LoRa module
    //PMU.setALDO3Voltage(3300);  //Set LDO for GPS module
    //PMU.setDC1Voltage(3300);    //Set DCDC for OLED
    //PMU.enableALDO2();          //Enable LDO2 for LoRa
    //PMU.enableALDO3();          //Enable LDO3 for GPS


    esp_reset_reason_t reason = esp_reset_reason();
    if (reason == ESP_RST_DEEPSLEEP) {
      long wakeup_source = esp_sleep_get_ext1_wakeup_status();
      if (wakeup_source & (1 << P_LORA_DIO_1)) {  // received a LoRa packet (while in deep sleep)
        startup_reason = BD_STARTUP_RX_PACKET;
      }

      rtc_gpio_hold_dis((gpio_num_t)P_LORA_NSS);
      rtc_gpio_deinit((gpio_num_t)P_LORA_DIO_1);
    }
  }

  void enterDeepSleep(uint32_t secs, int pin_wake_btn = -1) {
    esp_sleep_pd_config(ESP_PD_DOMAIN_RTC_PERIPH, ESP_PD_OPTION_ON);

    // Make sure the DIO1 and NSS GPIOs are hold on required levels during deep sleep
    rtc_gpio_set_direction((gpio_num_t)P_LORA_DIO_1, RTC_GPIO_MODE_INPUT_ONLY);
    rtc_gpio_pulldown_en((gpio_num_t)P_LORA_DIO_1);

    rtc_gpio_hold_en((gpio_num_t)P_LORA_NSS);

    if (pin_wake_btn < 0) {
      esp_sleep_enable_ext1_wakeup( (1L << P_LORA_DIO_1), ESP_EXT1_WAKEUP_ANY_HIGH);  // wake up on: recv LoRa packet
    } else {
      esp_sleep_enable_ext1_wakeup( (1L << P_LORA_DIO_1) | (1L << pin_wake_btn), ESP_EXT1_WAKEUP_ANY_HIGH);  // wake up on: recv LoRa packet OR wake btn
    }

    if (secs > 0) {
      esp_sleep_enable_timer_wakeup(secs * 1000000);
    }

    // Finally set ESP32 into sleep
    esp_deep_sleep_start();   // CPU halts here and never returns!
  }

  uint16_t getBattMilliVolts() override {
    return 0;
  }

  const char* getManufacturerName() const override {
    return "LilyGo T-Beam S3 Supreme SX1262";
  }
};
