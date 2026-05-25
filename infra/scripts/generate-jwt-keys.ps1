$ErrorActionPreference = "Stop"

$ScriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
$SecretsDir = Join-Path $ScriptDir "..\secrets"
New-Item -ItemType Directory -Force -Path $SecretsDir | Out-Null

$PrivateKey = Join-Path $SecretsDir "jwt_private.pem"
$PublicKey = Join-Path $SecretsDir "jwt_public.pem"

openssl genrsa -out $PrivateKey 2048
openssl rsa -in $PrivateKey -pubout -out $PublicKey

Write-Host "JWT RSA 키 페어를 infra/secrets/에 생성했습니다."
