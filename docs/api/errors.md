# 공통 에러

모든 API는 [응답 envelope 표준](envelope.md)을 따른다. 성공 시 `{ "ok": true, "data": ... }`, 실패 시 `{ "ok": false, "error": { "code", "message", "details?", "requestId?" } }` 형태를 사용한다.

| HTTP | 코드 | 기본 메시지 | 의미 |
| --- | --- | --- | --- |
| 400 | `VALIDATION_ERROR` | 요청 값이 올바르지 않습니다. | JSON Schema 또는 도메인 검증 실패 |
| 401 | `AUTH_ERROR` | 인증에 실패했습니다. | access/refresh 토큰 문제 또는 비밀번호 불일치 |
| 404 | `NOT_FOUND` | 대상을 찾을 수 없습니다. | 사용자 소유 리소스가 없거나 접근 불가 |
| 409 | `CONFLICT` | 이미 존재하는 리소스입니다. | 이메일, 닉네임 등 unique 충돌 |
| 429 | `RATE_LIMITED` | 요청 한도를 초과했습니다. | endpoint별 rate limit 초과 |
| 500 | `INTERNAL_ERROR` | 서버 오류가 발생했습니다. | 처리되지 않은 서버 오류 |

## 1.0 에러 코드 prefix

현재 백엔드 V1은 위 공통 코드 중심으로 구현되어 있다. 운영자 페이지와 클라이언트 UI는 아래 prefix를 기준으로 세부 코드가 추가될 수 있도록 준비한다.

| Prefix | 예시 코드 | 의미 |
| --- | --- | --- |
| `AUTH_*` | `AUTH_INVALID_CREDENTIALS`, `AUTH_TOKEN_EXPIRED`, `AUTH_RATE_LIMITED` | 인증, 토큰, 로그인 제한 |
| `SAVE_*` | `SAVE_PAYLOAD_INVALID`, `SAVE_CONFLICT_REJECTED`, `SAVE_CHARACTER_NOT_FOUND` | 세이브 업로드/다운로드, 서버 검증 |
| `LEADERBOARD_*` | `LEADERBOARD_TYPE_INVALID`, `LEADERBOARD_RANGE_INVALID` | 랭킹 타입, limit/range 오류 |
| `CHARACTER_*` | `CHARACTER_CLASS_UNAVAILABLE`, `CHARACTER_NOT_FOUND` | 캐릭터 생성/조회 |
| `SYSTEM_*` | `SYSTEM_HEALTH_DEGRADED`, `SYSTEM_INTERNAL` | 헬스체크, 서버 내부 오류 |

## 운영자 페이지 표시 매핑

| 분류 | 색상 토큰 | 아이콘 | 적용 예 |
| --- | --- | --- | --- |
| Critical | `--error-critical` | `alert-triangle` | `INTERNAL_ERROR`, `SYSTEM_*` 장애 |
| Warning | `--error-warn` | `circle-alert` | `RATE_LIMITED`, `SAVE_CONFLICT_REJECTED` |
| Info | `--error-info` | `info` | `VALIDATION_ERROR`, `CHARACTER_CLASS_UNAVAILABLE` |
| Auth | `--error-auth` | `shield-alert` | `AUTH_*` |

## 정합 메모

- PR #2 백엔드 V1의 실제 서버 코드는 `AUTH_ERROR`, `VALIDATION_ERROR`, `NOT_FOUND`, `CONFLICT`, `RATE_LIMITED`, `INTERNAL_ERROR`를 반환한다.
- 세부 prefix 코드로의 변경은 클라이언트 분기 영향이 있으므로 후속 fix 단계 또는 별도 PR에서 서버 코드, API 문서, QA 시나리오를 함께 갱신한다.
## PR #2 서버 반환 코드

서버 구현은 공통 클래스의 기본 코드 대신 도메인 prefix 코드를 명시적으로 반환한다.

| HTTP | 코드 | 상황 |
| --- | --- | --- |
| 400 | `SAVE_VALIDATION_FAILED` | 세이브 payload 권위 검증 실패 |
| 400 | `CHARACTER_CLASS_UNAVAILABLE` | MVP 범위 밖 classId 요청 |
| 401 | `AUTH_INVALID_CREDENTIALS` | 로그인 이메일/비밀번호 불일치 |
| 401 | `AUTH_ACCESS_TOKEN_INVALID` | access token 누락 또는 검증 실패 |
| 401 | `AUTH_REFRESH_TOKEN_INVALID` | refresh 엔드포인트에 refresh typ이 아닌 토큰 사용 |
| 401 | `AUTH_REFRESH_TOKEN_REVOKED` | 이미 폐기된 refresh token 사용 |
| 401 | `AUTH_REFRESH_TOKEN_INACTIVE` | Redis active jti가 없는 refresh token 사용 |
| 404 | `SAVE_CHARACTER_NOT_FOUND` | 세이브 대상 캐릭터 없음 또는 소유권 불일치 |
| 404 | `CHARACTER_NOT_FOUND` | 캐릭터 없음 또는 소유권 불일치 |
| 409 | `AUTH_EMAIL_ALREADY_EXISTS` | 이미 가입된 이메일 |
| 429 | `AUTH_RATE_LIMITED` | auth rate limit 초과 |
| 500 | `SYSTEM_INTERNAL` | 처리되지 않은 서버 오류 |
