/*
 *
 * Twilio Breakout Massive SDK
 *
 * Copyright (c) 2019 Twilio, Inc.
 *
 * SPDX-License-Identifier:  Apache-2.0
 */

/**
 * \file config.h - defines configuration parameters. Can be modified by the user to adapt to a particular use-case
 *
 */
#ifndef BREAKOUT_CONFIG_H_
#define BREAKOUT_CONFIG_H_

#include "tls_credentials.h"

#if defined(TLS_DEVICE_CERT) && defined(TLS_DEVICE_PKEY) && defined(TLS_SERVER_CA)
#  define USE_TLS true
#else
#  define USE_TLS false
#endif

#define MQTT_BROKER_HOST "mqtt.example.com"
#define MQTT_BROKER_PORT 8883
#define MQTT_KEEP_ALIVE 0
#define MQTT_CLIENT_ID "alfa-kit"
#define MQTT_PUBLISH_TOPIC "temperature"
#define MQTT_STATE_TOPIC "state"
#define MQTT_LOGIN NULL
#define MQTT_PASSWORD NULL

// TLS_PROFILE_ID 0 is usually a good default unless using multiple profiles - possible values 0-4
#define TLS_PROFILE_ID 0
#define TLS_CIPHER_SUITE USECPREF_CIPHER_SUITE_TLS_RSA_WITH_AES_256_CBC_SHA256

#endif // BREAKOUT_CONFIG_H_
