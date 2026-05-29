# 던전 난이도 티어 (Dungeon Difficulty Tiers) — 설계 문서

> 작성일: 2026-05-29 · 대상: PR #75 · 브랜치: `plan/75-dungeon-tiers`
> 분류: PR #68 던전 시스템 후속(엔드게임 반복 깊이) · PM 자율 진행

---

## 0. 한 줄 요약

각 일일 던전(Gold/Exp/Essence, #68)에 **무한 난이도 티어**를 추가한다. 높은
티어는 더 높은 전투력(CP)을 요구하고 더 큰 보상을 준다. 티어 접근성은 **CP에서
런타임 파생**(영구 상태 없음 → 세이브 변경 없음).

---

## 1. 목적 / 배경

- #68 던전은 타입별 단일 난이도(sqrt(CP/minCp) 보상)뿐 — 고CP 고수에게 선택·도전
  축이 없다. 티어로 **CP 성장이 던전 보상에 직접 반영**되는 무한 도전 추가.
- 무한 성장([[project-infinite-growth]]) — 티어는 CP에 따라 무한 해금.
- #49 CP·#68 던전·#74 심연 마스터리(던전 보상 +%)와 자연 연동.

---

## 2. 핵심 결정 (PM 자율)

| 항목 | 결정 | 근거 |
| --- | --- | --- |
| 티어 모델 | 타입별 **티어 1..N 무한**, CP 게이트 | 무한 성장·선택 축 |
| 접근성 | **CP 런타임 파생**(`CP ≥ 티어 요구치`) | 영구 상태 불필요 → **세이브 변경 없음**(#74처럼 안전) |
| 입장 제한 | 타입별 일일 3회 공유(티어 무관) | #68 구조 유지 |
| 티어 선택 | 플레이어가 해금된 티어 중 선택 | 위험/보상 트레이드오프 |

---

## 3. 티어 공식

```
TierCpFactor    = 2.0   // 티어당 CP 요구 배수
TierRewardStep  = 1.0   // 티어당 보상 배수 증가(선형: 티어 T → ×T)
MaxConsideredTier = 너무 큰 값 방지용 상한 없이 CP로 자연 제한(접근 가능 최고 티어 = CP로 산출)

GetTierCpRequirement(type, tier) = getMinimumCp(type) * TierCpFactor^(tier-1)   // tier>=1
GetMaxAccessibleTier(type, cp)  = CP ≥ 요구치를 만족하는 최대 tier (>=1, CP<minCp면 0)
GetDungeonReward(type, cp, tier):
  tier 접근 불가(cp < 요구치)면 0 보상.
  기존 sqrt(CP/minCp) 보상 × TierRewardMultiplier(tier),  TierRewardMultiplier(T)=T
```

- 무한: CP가 오를수록 더 높은 티어 해금 + 보상 무한 증가(단 입장 3회/일 제한으로 페이싱).
- 티어 1은 #68 기존 보상과 동일(`TierRewardMultiplier(1)=1`) → **회귀 안전**.

---

## 4. 통합 지점 (구현 개요)

| 파트 | 작업 |
| --- | --- |
| character (메인) | 클라 던전 공식(`FDungeonFormula` 또는 DungeonService 내)에 tier 파라미터, `UDungeonService::TryRunDungeon(Type, CP, TodayUtc, Tier)` + `GetMaxAccessibleTier`/`GetTierCpRequirement`, GameInstance `TryRunDungeon(Type, Tier)`, 클라 Automation. **세이브 변경 없음** |
| backend | `dungeon.ts`에 `getTierCpRequirement`/`getMaxAccessibleTier`/`getDungeonReward(type, cp, tier)` 미러 + vitest parity |
| designer | 던전 패널 티어 선택 UI(해금/잠금 티어, 요구 CP, 예상 보상) + ko/en |
| balance | TierCpFactor/RewardStep 근거 + 티어별 CP/보상 예시 + 무한 비폭주(입장 제한 페이싱) 문서 |
| qa | 티어 게이트(CP 미달 0)·보상 스케일·티어1 회귀·일일 입장 공유·parity 시나리오 |

**세이브: 변경 없음**(SaveVersion 14 유지 — 티어는 CP 런타임 파생, 입장 횟수는 #68 기존 저장).

---

## 5. 스코프

**In Scope (V1):** 타입별 무한 티어(CP 게이트), 티어 보상 배수, 티어 선택 UI,
서버 미러+parity, balance 문서, 5-team.

**Out of Scope (후속):** 티어별 고유 추가 보상(장비/소비 드롭), 주간 보스 던전,
티어별 입장 제한 분리, 최고 티어 도달 업적/연출.

---

## 6. 리스크

| 리스크 | 완화 |
| --- | --- |
| 티어 보상 무한 폭주 | 입장 3회/일 제한 + 선형 보상(geometric 회피) + balance 문서 |
| CP 게이트 부정확(과해금) | `GetMaxAccessibleTier` 경계 테스트(CP=요구치±1) + parity |
| #68 회귀 | 티어1 = 기존 보상 동일(`×1`) + 기존 호출부 기본 티어1 + 회귀 테스트 |
| 서버↔클라 drift | `dungeon.ts` Math.fround 미러 + 경계 parity |
| 마스터리 심연(#74)·CP 합성 | 던전 보상 = 티어배수 후 #74 심연 로컬(+%) 적용 순서 명확화, 단일 적용 |

---

## 7. 워크플로우 v3 / 후속

[1] PM 기획+PR → [2] Codex 5-team → [3] Claude TM → [4] Codex(결함 시) → [5] 검증(UE Automation 직접) → [N] CI 그린 + PM 종합 + 머지. PM 자율.

**후속:** 티어별 고유 보상, 주간 보스 던전, 최고 티어 업적/연출.
