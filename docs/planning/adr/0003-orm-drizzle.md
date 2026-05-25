# ADR-0003 ORM: Drizzle 채택

## 상태

채택

## 맥락

TypeScript 서버에서 PostgreSQL 접근 계층이 필요하다. 후보는 Drizzle과 Prisma였다.

## 결정

Drizzle ORM과 명시적 SQL 마이그레이션을 채택한다.

## 이유

Drizzle은 가볍고 SQL과 가까워 마이그레이션 결과를 리뷰하기 쉽다. Prisma는 개발 경험이 좋지만 생성 클라이언트와 자체 migration layer가 커지고, 이 PR처럼 SQL 파일과 문서의 1:1 대응을 요구하는 범위에서는 Drizzle이 더 명료하다.
