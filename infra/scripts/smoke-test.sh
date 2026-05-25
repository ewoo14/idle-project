#!/usr/bin/env bash
set -euo pipefail

BASE_URL="${BASE_URL:-http://localhost:3000}"
STAMP="$(date +%s)"
EMAIL="smoke-${STAMP}@idle.local"
PASSWORD="Password123!"
NICKNAME="smoke${STAMP: -5}"

REGISTER=$(curl -sS -X POST "${BASE_URL}/v1/auth/register" -H "content-type: application/json" \
  -d "{\"email\":\"${EMAIL}\",\"password\":\"${PASSWORD}\",\"nickname\":\"${NICKNAME}\"}")
ACCESS=$(node -e "const r=JSON.parse(process.argv[1]); console.log(r.data.accessToken)" "$REGISTER")

CHARACTER=$(curl -sS -X POST "${BASE_URL}/v1/characters" -H "authorization: Bearer ${ACCESS}" -H "content-type: application/json" -d '{"classId":1}')
CHARACTER_ID=$(node -e "const r=JSON.parse(process.argv[1]); console.log(r.data.id)" "$CHARACTER")

curl -sS -X PUT "${BASE_URL}/v1/save" -H "authorization: Bearer ${ACCESS}" -H "content-type: application/json" \
  -d "{\"characterId\":\"${CHARACTER_ID}\",\"version\":1,\"payload\":{\"level\":1,\"rebirthCount\":0,\"maxEquipmentGrade\":0}}" >/dev/null

curl -sS "${BASE_URL}/v1/leaderboard/power?season=1&limit=10" >/dev/null
echo "smoke test passed"
