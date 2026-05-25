# 공통 에러

모든 API는 성공 시 `{ "ok": true, "data": ... }`, 실패 시 `{ "ok": false, "error": { "code", "message", "requestId" } }` 형태를 사용한다.

| HTTP | 코드 | 기본 메시지 | 의미 |
| --- | --- | --- | --- |
| 400 | `VALIDATION_ERROR` | 요청 값이 올바르지 않습니다. | JSON Schema 또는 도메인 검증 실패 |
| 401 | `AUTH_ERROR` | 인증에 실패했습니다. | access/refresh 토큰 문제 또는 비밀번호 불일치 |
| 404 | `NOT_FOUND` | 대상을 찾을 수 없습니다. | 사용자 소유 리소스가 없거나 접근 불가 |
| 409 | `CONFLICT` | 이미 존재하는 리소스입니다. | 이메일, 닉네임 등 unique 충돌 |
| 429 | `RATE_LIMITED` | 요청 한도를 초과했습니다. | endpoint별 rate limit 초과 |
| 500 | `INTERNAL_ERROR` | 서버 오류가 발생했습니다. | 처리되지 않은 서버 오류 |
