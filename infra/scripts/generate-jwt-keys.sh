#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
SECRETS_DIR="${SCRIPT_DIR}/../secrets"
mkdir -p "${SECRETS_DIR}"

openssl genrsa -out "${SECRETS_DIR}/jwt_private.pem" 2048
openssl rsa -in "${SECRETS_DIR}/jwt_private.pem" -pubout -out "${SECRETS_DIR}/jwt_public.pem"
chmod 600 "${SECRETS_DIR}/jwt_private.pem"
chmod 644 "${SECRETS_DIR}/jwt_public.pem"

echo "JWT RSA 키 페어를 infra/secrets/에 생성했습니다."
