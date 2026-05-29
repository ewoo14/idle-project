# 주간 보스 (Weekly Boss) — 설계 문서

> 작성일: 2026-05-29 · 대상: PR #77 · 브랜치: `plan/77-weekly-boss` · PM 자율 진행
> 분류: 신규 엔드게임 반복 콘텐츠 (주간 cadence)

---

## 0. 한 줄 요약

매주 리셋되는 보스에게 **주간 누적 데미지**(CP 기반 도전, 주당 횟수 제한)를 쌓아
**무한 마일스톤 보상**을 받는다. 일일 던전(#68/#75)·영구 탑(#50/#51)과 다른
**주간 cadence** 리텐션 루프.

---

## 1. 목적 / 배경

- 현 반복 콘텐츠: 일일(던전), 영구(탑). **주간 단위 목표가 없음** — 주간 cadence는
  강력한 리텐션 벡터.
- 무한 성장([[project-infinite-growth]]): CP↑ → 도전 데미지↑ → 더 높은 마일스톤.
- 재활용: 주간 리셋(`UQuestService::GetCurrentUtcWeekString` ISO week, #56), CP(#49),
  마일스톤 보상(탑 #51 패턴), 던전 도전 제한(#68).
- 추후 #76 리더보드에 "주간 보스 데미지" 랭킹으로 확장 가능(후속).

---

## 2. 핵심 결정 (PM 자율)

| 항목 | 결정 | 근거 |
| --- | --- | --- |
| 루프 | 주간 누적 데미지 → 무한 마일스톤 보상 | 주간 목표 + 무한 성장 |
| 도전 | 주당 횟수 제한(7회), 각 데미지=CP 기반 | 페이싱 + CP 반영 |
| 리셋 | ISO week 문자열(새 주 진입 시 누적/도전/수령 리셋) | 기존 주간 패턴 재사용 |
| 마일스톤 | 무한 기하 임계(`base×growth^n`), 도달분 수령 | 무한 성장·체감 |
| 보상 | 골드/에센스(+소비 아이템 #73 가능), 마일스톤 비례 | 기존 재화 재사용 |
| 세이브 | **14→15**(누적 데미지/주 id/도전 사용/수령 마일스톤) | 신규 영속 상태 |

---

## 3. 공식 / 흐름

```
WEEKLY_CHALLENGE_LIMIT = 7
GetChallengeDamage(combatPower) = max(0, combatPower)              // 1회 도전 데미지(CP)
MilestoneThreshold(n) = floor(1000 * 1.5^(n-1))   // n>=1, 무한 기하
GetReachedMilestones(accumDamage) = accumDamage>=threshold(n)인 최대 n (없으면 0)
MilestoneReward(n): 골드/에센스 = base * n (또는 기하) — balance 확정
```

흐름:
1. **도전**: `TryChallengeWeeklyBoss()` — 새 주면 리셋(누적 0/도전 0/수령 0/주 id 갱신). 도전 잔여>0이면 `AccumDamage += GetChallengeDamage(CP)`, `ChallengesUsed++`.
2. **수령**: `ClaimWeeklyBossMilestone(n)` — `n<=GetReachedMilestones(AccumDamage)` 이고 미수령이면 `MilestoneReward(n)` 지급 + 수령 표시.
3. **리셋**: 새 ISO week 진입 시 누적/도전/수령 초기화(주 id 갱신).

---

## 4. 통합 지점 (구현 개요)

| 파트 | 작업 |
| --- | --- |
| character (메인) | `WeeklyBossTypes.h`(`FWeeklyBossSaveState`/`FWeeklyBossChallengeResult`), `FWeeklyBossFormula`(+서버 `weeklyBoss.ts` 미러), `UWeeklyBossService`(누적/도전/수령/주리셋), GameInstance(`TryChallengeWeeklyBoss`/`ClaimWeeklyBossMilestone`/접근자), 세이브 14→15, 클라 Automation |
| backend | `weeklyBoss.ts`(GetChallengeDamage/MilestoneThreshold/GetReachedMilestones/MilestoneReward) 미러 + vitest parity + save.schema 선택 필드 |
| designer | 주간 보스 패널(진행=누적 vs 다음 마일스톤, 도전 잔여/버튼, 마일스톤 목록/수령, 주 리셋 표시) + ko/en |
| balance | 도전 제한·데미지·마일스톤 곡선·보상 + 무한 비폭주 + 주간 cadence 페이싱 문서 |
| qa | 도전/누적/마일스톤 수령(중복 방지)/주 리셋/세이브 14→15/회귀 + parity |

### 4.1 세이브
- SaveVersion **14→15**. 신규: `WeeklyBossWeekId`(FString), `WeeklyBossDamage`(int64), `WeeklyBossChallengesUsed`(int32), `WeeklyBossClaimedMilestones`(int32 = 수령한 최고 n 또는 count). v14 로드 시 빈/0 마이그레이션. 클라우드 payload. 환생/초월 리셋과 무관(주간 자체 리셋).

---

## 5. 스코프

**In Scope (V1):** 주간 보스 1종, CP 도전(주 7회), 무한 마일스톤 보상, 주 리셋,
패널 UI, 세이브 14→15, 서버 미러+parity, balance, 5-team.

**Out of Scope (후속):** 주간 보스 데미지 리더보드(#76 연동), 보스 페이즈/속성 연출(#35),
다중 주간 보스 로테이션, 길드 협력 보스.

---

## 6. 리스크

| 리스크 | 완화 |
| --- | --- |
| 마일스톤 보상 폭주 | 주 7회 제한 + 기하 임계(고마일스톤 도달 어려움) + balance |
| 주 리셋 경계 정확성 | ISO week 문자열 비교(#56 검증된 패턴) + 경계 Automation |
| jumbo(unity) 빌드 ODR(#76 교훈) | 신규 익명 namespace/헬퍼 동명 grep 선확인, 표준 jumbo 빌드 PM 검증 |
| 세이브 14→15 회귀 | 누락=0 마이그레이션 + 라운드트립 + 클라우드 payload |
| 서버↔클라 drift | `weeklyBoss.ts` Math.fround 미러 + 경계 parity |
| 또 다른 CP-보상 루프 중복감 | 주간 cadence·누적·리셋으로 일일/영구와 차별 |

---

## 7. 워크플로우 v3 / 후속

[1] PM 기획+PR → [2] Codex 5-team → [3] Claude TM → [4] Codex(결함 시) → [5] 검증(**표준 jumbo(unity) 빌드 PM 직접**, #76 교훈) → [N] CI 그린 + PM 종합 + 머지. PM 자율.

**후속:** 주간 보스 데미지 리더보드(#76), 보스 페이즈 연출, 다중 보스 로테이션, 길드 협력.
