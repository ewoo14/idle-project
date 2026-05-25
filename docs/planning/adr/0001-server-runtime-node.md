# ADR-0001 서버 런타임: Node.js 채택

## 상태

채택

## 맥락

백엔드 V1은 인증, 세이브, 리더보드, 캐릭터 API를 빠르게 제공해야 한다. 후보는 Node.js와 Go였다.

## 결정

Node.js 22 LTS와 TypeScript 5.5를 채택한다.

## 이유

TypeScript는 JSON Schema, 클라이언트 계약, 테스트 작성 속도 면에서 이 프로젝트의 에이전트 기반 구현에 유리하다. Fastify 5는 스키마 검증과 플러그인 생태계가 성숙했고, Redis/JWT/PostgreSQL 연동도 검증된 패키지가 많다. Go는 런타임 효율이 장점이지만 현재 범위에서는 개발 속도와 문서화 일관성이 더 중요하다.
