/*
 *
 * Twilio Breakout Massive SDK
 *
 * Copyright (c) 2019 Twilio, Inc.
 *
 * SPDX-License-Identifier:  Apache-2.0
 */

/**
 * \file tls_credentials.h - TLS credentials to use in your application.
 *
 */

#ifndef TLS_CREDENTIALS_H_
#define TLS_CREDENTIALS_H_

// For best results, provide a DER encoded CA root, device certificate and device private key here
// PEM is supported, but the MD5 calculation will result in the items being updated every time.  If
// you provide DER encoded items here, the MD5 sums matching the contents of the module will result
// in fewer NVM writes.
//
// To generate this file from *.pem or *.der files, use the script in the 'scripts' directory:
//
//   ./scripts/make_tls_credentials.sh cert.pem pkey.pem ca.pem > src/tls_credentials.h

//#define TLS_SERVER_CA "\x00\x01\x02..."

//#define TLS_DEVICE_CERT "\x00\x01\x02..."

//#define TLS_DEVICE_PKEY "\x00\x01\x02.."

#endif // TLS_CREDENTIALS_H_
