# ADR-0002 데이터베이스: PostgreSQL 채택

## 상태

채택

## 맥락

세이브 데이터는 JSON 구조를 포함하지만 사용자, 캐릭터, 리더보드는 관계와 트랜잭션이 중요하다. 후보는 PostgreSQL과 MongoDB였다.

## 결정

PostgreSQL 16을 기본 영속 저장소로 채택한다.

## 이유

사용자/캐릭터/리더보드는 명확한 관계형 제약과 인덱스가 필요하다. PostgreSQL은 JSONB로 세이브 payload를 유연하게 저장하면서도 FK, unique, transaction을 제공한다. MongoDB는 JSON 문서 저장은 편하지만 계정/캐릭터/리더보드 정합성 보장과 SQL 기반 운영 검증에서 불리하다.
