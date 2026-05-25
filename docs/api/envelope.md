# API 응답 Envelope 표준

서버와 클라이언트는 모든 HTTP JSON 응답을 아래 envelope로 주고받는다.

## 성공 응답

```json
{
  "ok": true,
  "data": {
    "id": "018f6f48-7c9b-7d1d-9c31-6d7342d44a20"
  }
}
```

- `ok`: 항상 `true`
- `data`: 엔드포인트별 성공 payload
- 성공 응답에는 `error`를 포함하지 않는다.

## 실패 응답

```json
{
  "ok": false,
  "error": {
    "code": "AUTH_INVALID_CREDENTIALS",
    "message": "이메일 또는 비밀번호가 잘못되었습니다.",
    "details": {
      "field": "password"
    },
    "requestId": "req-018f6f48"
  }
}
```

- `ok`: 항상 `false`
- `error.code`: 클라이언트 분기용 안정 코드
- `error.message`: 사용자 또는 운영자에게 노출 가능한 한글 메시지
- `error.details`: 선택 필드. 필드 검증, rate limit 남은 시간 등 구조화된 부가 정보
- `error.requestId`: 선택 필드. 서버 로그 추적용 요청 ID

## 공통 직렬화 규칙

| 항목 | 표준 |
| --- | --- |
| 시간 | ISO 8601 UTC. 예: `"createdAt": "2026-05-25T03:00:00.000Z"` |
| ID | UUID v7 문자열 |
| 필드명 | 서버 ↔ 클라이언트 모두 **camelCase** |
| 빈 목록 | `[]` |
| 없는 단일 리소스 | 성공 envelope 안에서는 `null`, 실패 상황에서는 해당 도메인 에러 |

## 현재 백엔드 정합 상태

PR #2 백엔드 V1은 이미 `{ ok: true, data }`, `{ ok: false, error }` envelope를 사용한다. 다만 에러 코드는 현재 `AUTH_ERROR`, `VALIDATION_ERROR`, `NOT_FOUND` 같은 공통 코드 중심이다. `AUTH_*`, `SAVE_*`, `LEADERBOARD_*`, `CHARACTER_*` 세부 prefix 정합 작업은 후속 fix 단계 또는 별도 PR에서 서버 코드와 함께 반영한다.
