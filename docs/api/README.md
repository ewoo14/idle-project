# API 문서

> 본 디렉터리는 백엔드 API 명세를 보관합니다. 백엔드 슬라이스 PR (PR #7) 시점에 본격 작성됩니다. 현재는 윤곽만.

## 구조 (예정)

| 파일 | 내용 |
| --- | --- |
| `envelope.md` | 공통 응답 envelope, 시간/ID/필드명 직렬화 표준 |
| `auth.md` | 인증 API (회원가입, 로그인, 토큰 갱신) |
| `save.md` | 클라우드 세이브 API |
| `character.md` | 캐릭터 / 환생 API |
| `offline.md` | 오프라인 보상 preview/claim API |
| `quest.md` | 퀘스트 목록/진행/보상 API |
| `leaderboard.md` | 랭킹 API |
| `errors.md` | 공통 에러 코드와 1.0 prefix 설계 |
| `events.md` | WebSocket 이벤트 (1.0) |
| `schema.md` | DB ERD + 테이블 |
| `openapi.yaml` | OpenAPI 3.1 스펙 (자동 생성) |

## 명세 작성 원칙

- 모든 엔드포인트: 메서드, 경로, 요청/응답 JSON Schema, 에러 코드, 권한, 예시
- 한글로 설명, 예시는 영문 키 + 한글 주석
- 변경 시 OpenAPI 자동 생성 / lint 통과

## 현재 윤곽

`docs/planning/02-architecture.md §3.3` 참고.
