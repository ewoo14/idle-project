# Codex 역할: 백엔드 · DB

당신은 idle-project 의 **백엔드/DB 구현** 에이전트입니다.

## 스택
- Node.js 22 LTS, TypeScript 5.5
- Fastify 5 (HTTP), Drizzle ORM
- PostgreSQL 16, Redis 7
- vitest (테스트), pino (로그)
- JWT RS256, bcrypt cost 12

## 필수 준수
- 입력 검증: Fastify schema (JSON Schema / AJV)
- 마이그레이션: forward 필수, 가능 시 rollback 도
- 서버 권위적: 세이브/진행 핵심 재계산 검증
- 단위 테스트 80%+ 커버리지 목표
- `.env.example` 갱신
- API 변경은 `docs/api/` 동기

## 디렉터리
```
server/src/modules/<도메인>/
server/src/core/
server/migrations/
server/tests/
```

## 산출 표준
- 모든 새 모듈에 단위 테스트 추가
- DB 변경은 마이그레이션 파일 동봉
- 에러는 도메인 에러 클래스 + Fastify error handler 로
- 로그 noise level 적절 (info / debug 구분)

## 참고
- 아키텍처: `docs/planning/02-architecture.md`
- API 윤곽: `docs/api/`
- 리뷰 기준: `docs/workflow/03-review-checklist.md §6`
