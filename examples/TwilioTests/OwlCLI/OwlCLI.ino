/*
 * OwlCLI.ino
 * Twilio Breakout SDK
 *
 * Copyright (c) 2018 Twilio, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/**
 * \file OwlModemTest.ino - Example for testing out functionality of the modem.
 */

#include <BreakoutSDK.h>

#include "CLI/OwlModemCLI.h"
#include "platform/ArduinoSeeedOwlSerial.h"

/* Modem result buffer */
#define MODEM_RESULT_LEN 256
char result_buffer[MODEM_RESULT_LEN];
str modem_result = {.s = result_buffer, .len = 0};

/**
 *  Replace here with your SIM card's PIN, or modify the handler_PIN() function below to get it from somewhere.
 *  Because the PIN can be binary, set correctly also the length in the str structure below.
 */
static str sim_pin = STRDECL("000000");

ArduinoSeeedUSBOwlSerial *debug_serial = 0;
ArduinoSeeedHwOwlSerial *module_serial = 0;
ArduinoSeeedHwOwlSerial *gnss_serial = 0;
/* This is only for API demonstration purposes, can be removed for in-production */
OwlModemCLI *owlModemCLI = 0;



void setup() {
  owl_log_set_level(L_INFO);
  LOG(L_NOTICE, "Arduino setup() starting up\r\n");

  debug_serial  = new ArduinoSeeedUSBOwlSerial(&SerialDebugPort);
  module_serial = new ArduinoSeeedHwOwlSerial(&SerialModule, SerialModule_Baudrate);
  gnss_serial = new ArduinoSeeedHwOwlSerial(&SerialGnss, SerialGnss_Baudrate);
  owlModemCLI   = new OwlModemCLI(owlModem, debug_serial);

  owlModemCLI->setup(TESTING_VARIANT_INIT, TESTING_VARIANT_REG, TESTING_APN, sim_pin);

  LOG(L_NOTICE, "Arduino setup() done\r\n");
  LOG(L_NOTICE, "Arduino loop() starting\r\n");
  return;
}


int cli_resume = 0;

void loop() {
  /* Take user input and run commands interactively - optional, if you want CLI */
  cli_resume = owlModemCLI->handleUserInput(cli_resume);

  /* Take care of async modem events */
  owlModem->AT.spin();
  /* Important step - handling UDP data */
  owlModem->socket.handleWaitingData();

  delay(50);
}



/*
 * Tests and examples for using the modem module
 */

void test_modem_bypass() {
  owlModem->bypass();
  delay(100);
}



void test_modem_get_info() {
  if (!owlModem->information.getProductIdentification(&modem_result)) {
    LOG(L_ERR, "Error retrieving Product Information\r\n");
  } else {
    LOG(L_INFO, "Product Information:\r\n%.*s\r\n--------------------\r\n", modem_result.len, modem_result.s);
  }

  if (!owlModem->information.getManufacturer(&modem_result)) {
    LOG(L_ERR, "Error retrieving Manufacturer Information\r\n");
  } else {
    LOG(L_INFO, "Manufacturer Information:\r\n%.*s\r\n-------------------------\r\n", modem_result.len, modem_result.s);
  }

  if (!owlModem->information.getModel(&modem_result)) {
    LOG(L_ERR, "Error retrieving ModelInformation\r\n");
  } else {
    LOG(L_INFO, "Model Information:\r\n%.*s\r\n-------------------------\r\n", modem_result.len, modem_result.s);
  }

  if (!owlModem->information.getVersion(&modem_result)) {
    LOG(L_ERR, "Error retrieving Version Information\r\n");
  } else {
    LOG(L_INFO, "Version Information:\r\n%.*s\r\n-------------------------\r\n", modem_result.len, modem_result.s);
  }



  if (!owlModem->information.getIMEI(&modem_result)) {
    LOG(L_ERR, "Error retrieving IMEI\r\n");
  } else {
    LOG(L_INFO, "IMEI: [%.*s]\r\n", modem_result.len, modem_result.s);
  }

  if (!owlModem->SIM.getICCID(&modem_result)) {
    LOG(L_ERR, "Error retrieving ICCID\r\n");
  } else {
    LOG(L_INFO, "ICCID: [%.*s]\r\n", modem_result.len, modem_result.s);
  }

  if (!owlModem->SIM.getIMSI(&modem_result)) {
    LOG(L_ERR, "Error retrieving IMSI\r\n");
  } else {
    LOG(L_INFO, "IMSI: [%.*s]\r\n", modem_result.len, modem_result.s);
  }

  if (!owlModem->SIM.getMSISDN(&modem_result)) {
    LOG(L_ERR, "Error retrieving IMSI\r\n");
  } else {
    LOG(L_INFO, "MSISDN: [%.*s]\r\n", modem_result.len, modem_result.s);
  }

  if (!owlModem->information.getBatteryChargeLevels(&modem_result)) {
    LOG(L_ERR, "Error retrieving Battery Charge Levels\r\n");
  } else {
    LOG(L_INFO, "Battery Charge Levels: [%.*s]\r\n", modem_result.len, modem_result.s);
  }

  if (!owlModem->information.getIndicatorsHelp(&modem_result)) {
    LOG(L_ERR, "Error retrieving Indicators-Help\r\n");
  } else {
    LOG(L_INFO, "Indicators-Help: [%.*s]\r\n", modem_result.len, modem_result.s);
  }

  if (!owlModem->information.getIndicators(&modem_result)) {
    LOG(L_ERR, "Error retrieving Indicators\r\n");
  } else {
    LOG(L_INFO, "Indicators: [%.*s]\r\n", modem_result.len, modem_result.s);
  }

  delay(5000);
}


void test_modem_network_management() {
  at_cfun_power_mode_e mode;

  LOG(L_INFO, "Retrieving modem functionality mode (AT+CFUN?)\r\n");
  if (!owlModem->network.getModemFunctionality(&mode, 0)) {
    LOG(L_ERR, "Error retrieving modem functionality mode\r\n");
    delay(500000000);
  }

  switch (mode) {
    case AT_CFUN__POWER_MODE__Minimum_Functionality:
      LOG(L_INFO, "Functionality is [Minimum] --> switching to [Full]\r\n");
      mode = AT_CFUN__POWER_MODE__Full_Functionality;
      break;
    case AT_CFUN__POWER_MODE__Full_Functionality:
      LOG(L_INFO, "Functionality is [Full] --> switching to [Airplane]\r\n");
      mode = AT_CFUN__POWER_MODE__Airplane_Mode;
      break;
    case AT_CFUN__POWER_MODE__Airplane_Mode:
      LOG(L_INFO, "Functionality is [Airplane] --> switching to [Minimum]\r\n");
      mode = AT_CFUN__POWER_MODE__Minimum_Functionality;
      break;
    default:
      LOG(L_ERR, "Unexpected functionality mode %d\r\n", mode);
      delay(500000000);
  }
  if (!owlModem->network.setModemFunctionality((at_cfun_fun_e)mode, 0)) {
    LOG(L_ERR, "Error setting modem functionality mode\r\n");
    delay(500000000);
  }

  delay(5000);
}
