# 길드 기초 PR-G3 QA 검증 노트

> 대상: PR-G3 (길드 보스 공유 HP 풀 + 주간 길드 랭킹) · 작성 2026-05-30 · 검증: PM
> **길드 시스템(백로그 #80) 완결.**

## 1. 자동 검증 게이트

| 게이트 | 범위 | 결과 |
| --- | --- | --- |
| 서버 vitest (`src/modules/guild`) | 도전 한도·누적·단일/연속 격파·데미지→기여·주간 리셋·랭킹 정렬/동점/내순위 | **63 passed** |
| 서버 lint/build | 전체 | **GREEN** |
| UE jumbo 빌드 | 전체(ODR) | **Succeeded** |
| UE Automation(클라) | GameCore(보스 parity·SaveSystem v19)/Combat/Localization/UI/Mastery/Consumable/Dungeon/Rune | **190/0** |
| UE Automation(UI) | GameCore/Localization(CsvIntegrity)/UI | **111/0** |

## 2. 시나리오 커버리지

- **길드 보스(공유 HP 풀)**: 멤버 도전(주당 7) → accum 공유 누적 → 격파(accum≥HP, defeated++, 잔여 이월), **연속 격파**(누적이 여러 HP 임계 한 번에 초과 시 defeated 다중 증가). HP 곡선 1M×1.5^.
- **보스 데미지→기여(4번째 발생원)**: `floor(dmg/10000)` → 서버 applyContribution 단일 적립(클라 이중적립 없음). 길드 레벨·상점·랭킹에 반영.
- **격파 보상**: 격파 N건 미수령분 누적(gold 200k + essence 5)/격파, claim 시 실존 재화 지급(ApplyGuildShopReward 재사용), 동시성 가드 멱등.
- **주간 길드 랭킹**: Σ weekly_contribution 길드별 정렬(동점 created_at), 내 길드 순위, 미소속.

## 3. 클라 Automation (서버 무의존)

- GuildBossFormula parity(HP 연속 격파 경계 defeated 0/1/2, getChallengeDamage), 보스 스냅샷 파싱/접근자, 보상 지급(gold/essence), 랭킹 파싱.
- 세이브 **v19 라운드트립**, 전 세이브 테스트 v19 갱신.

## 4. PM 설계 교정 (기록)

- 보스 보상이 `contributionPoints`(서버 컬럼 — 클라 지급 불가)였던 것을 **essence(클라 적용 가능)로 교정**(G2 죽은재화 교훈 연속, 작동하는 슬라이스).

## 5. 한계 / 후속

- cross-DB 정합 SQL(0011 마이그 up/down + 보스/랭킹 PG 쿼리)은 라이브 DB 미기동으로 통합/스테이징 후속.
- 주간 랭킹 **보상**은 본 PR 범위 밖(표시까지) → 시즌 보상(#78 패턴) 후속.
- 보스 HP·도전 한도·데미지→기여 환산·보상 수치는 초기값 → 활동 데이터 재튜닝. PR 본문 보스/랭킹 스크린샷.
