#!/bin/bash

# Generates src/tls_credentials.h files from the TLS triple: client certificate,
#  client private key and server CA certificate. Both PEM and DER formats are
#  supported.

pem_to_c_der_cert() {
  openssl x509 -in $1 -outform der | xxd -p | tr -d "\r\n" | sed -n 's/\(..\)/\\x\1/pg'
}

pem_to_c_der_rsa() {
  openssl rsa -in $1 -outform der | xxd -p | tr -d "\r\n" | sed -n 's/\(..\)/\\x\1/pg'
}

# Prints non-empty string if PEM
is_pem() {
  cat $1 | head -n 1 | sed -n "/^-----BEGIN/p"
}

if [ $# -ne 3 ]; then
	echo "Usage: make_tls_credentials.sh [client_cert.pem | client_cert.der] [client_pkey.pem | client_pkey.der] [server_cacert.pem | server_cacert.der]" >&2
	exit 1
fi

echo "#ifndef TLS_CREDENTIALS_H_"
echo "#define TLS_CREDENTIALS_H_"
echo ""
echo -n "#define TLS_DEVICE_CERT \""

if [ -n "$(is_pem $1)" ]; then
  contents=$(pem_to_c_der_cert $1)
else
  contents=$(cat $1)
fi

echo -n ${contents}

echo -e "\"\n"

echo -n "#define TLS_DEVICE_PKEY \""

if [ -n "$(is_pem $2)" ]; then
  contents=$(pem_to_c_der_rsa $2)
else
  contents=$(cat $2)
fi

echo -n ${contents}

echo -e "\"\n"

echo -n "#define TLS_SERVER_CA \""

if [ -n "$(is_pem $3)" ]; then
  contents=$(pem_to_c_der_cert $3)
else
  contents=$(cat $3)
fi

echo -n ${contents}

echo -e "\"\n"
echo "#endif // TLS_CREDENTIALS_H_"
