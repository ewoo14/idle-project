# M3 백엔드 V1 QA 시나리오

> 대상: PR #2 백엔드 V1 산출물. 자동화 도구는 현재 서버 테스트 구조를 기준으로 분류한다.

## 1. 회원가입 정상

- **도구**: vitest 통합
- **Given** 신규 이메일, 닉네임, 비밀번호가 준비되어 있다.
- **When** `POST /v1/auth/register`를 호출한다.
- **Then** `{ ok: true, data: { user, accessToken, refreshToken } }` 응답을 받고 사용자가 생성된다.

## 2. 로그인 정상

- **도구**: vitest 통합
- **Given** 가입된 사용자 계정이 있다.
- **When** 올바른 이메일과 비밀번호로 `POST /v1/auth/login`을 호출한다.
- **Then** access token과 refresh token이 발급된다.

## 3. 로그인 실패 — 잘못된 비밀번호

- **도구**: vitest 통합
- **Given** 가입된 사용자 계정이 있다.
- **When** 잘못된 비밀번호로 `POST /v1/auth/login`을 호출한다.
- **Then** `401`과 `AUTH_*` 계열 에러 응답을 받으며 토큰은 발급되지 않는다.

## 4. 로그인 실패 — rate limit

- **도구**: E2E smoke
- **Given** 동일 IP에서 로그인 요청을 반복할 수 있다.
- **When** 제한 횟수를 초과해 `POST /v1/auth/login`을 호출한다.
- **Then** `429`와 rate limit 에러 응답을 받는다.

## 5. 토큰 갱신 정상

- **도구**: vitest 통합
- **Given** 유효한 refresh token이 있다.
- **When** `POST /v1/auth/refresh`를 호출한다.
- **Then** 새 access token이 발급되고 응답 envelope가 유지된다.

## 6. 세이브 업로드 정상

- **도구**: vitest 통합
- **Given** 사용자 소유 캐릭터가 있고 payload의 `level`, `rebirthCount`, `maxEquipmentGrade`가 서버 검증 범위 안에 있다.
- **When** `POST /v1/saves`를 호출한다.
- **Then** 세이브가 저장되고 `serverValidated=true`가 기록된다.

## 7. 세이브 업로드 실패 — 조작된 payload reject

- **도구**: vitest 통합
- **Given** 사용자 소유 캐릭터의 서버 레벨과 환생 수가 저장되어 있다.
- **When** 레벨 캡 초과, 환생 수 감소, 장비 강화 수치 범위 초과 payload를 업로드한다.
- **Then** `400 VALIDATION_ERROR`로 거절되고 세이브가 저장되지 않는다.

## 8. 세이브 다운로드

- **도구**: vitest 통합
- **Given** 해당 캐릭터에 저장된 세이브 이력이 있다.
- **When** `GET /v1/saves/:characterId/current`를 호출한다.
- **Then** 가장 최근 세이브 1건이 `{ ok: true, data }`로 반환된다.

## 9. 리더보드 조회

- **도구**: vitest 통합
- **Given** 여러 캐릭터의 power 또는 rebirth 값이 저장되어 있다.
- **When** `GET /v1/leaderboards/power`, `GET /v1/leaderboards/rebirth`를 호출한다.
- **Then** 정렬된 상위 목록이 반환되고 limit 범위가 보정된다.

## 10. 캐릭터 생성 — 전사

- **도구**: vitest 통합
- **Given** 인증된 사용자가 있다.
- **When** `POST /v1/characters`에 `{ "classId": 1 }`을 보낸다.
- **Then** level 1, rebirth 0, 전사 `stats.primary`와 `stats.derived`가 포함된 캐릭터가 생성된다.

## 11. 헬스체크

- **도구**: E2E smoke
- **Given** 서버 프로세스가 기동되어 있다.
- **When** `GET /healthz`를 호출한다.
- **Then** `{ ok: true, data: { status: "healthy" } }` 응답을 받는다.

## 12. 실패 회복 — 로그아웃 후 재로그인

- **도구**: 수동
- **Given** 로그인된 세션과 refresh token이 있다.
- **When** `POST /v1/auth/logout` 후 같은 계정으로 다시 로그인한다.
- **Then** 기존 refresh token은 폐기되고 새 토큰 세트로 정상 API 호출이 가능하다.
