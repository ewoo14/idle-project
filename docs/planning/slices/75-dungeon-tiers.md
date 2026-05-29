# PR #75 기획서 — 던전 난이도 티어 (Dungeon Difficulty Tiers)

> **PM 자율 진행**. PR #68 던전 시스템 후속(엔드게임 반복 깊이). 각 일일 던전(Gold/Exp/Essence)에 **무한 난이도 티어** 추가 — 높은 티어는 더 높은 전투력(CP)을 요구하고 더 큰 보상. 티어 접근성은 CP 런타임 파생(**세이브 변경 없음**). 무한 성장([[project-infinite-growth]]). client + server 5-team.
>
> 스펙: [`docs/superpowers/specs/2026-05-29-dungeon-tiers-design.md`](../../superpowers/specs/2026-05-29-dungeon-tiers-design.md)
> 계획(TDD): [`docs/superpowers/plans/2026-05-29-dungeon-tiers.md`](../../superpowers/plans/2026-05-29-dungeon-tiers.md)

## 1. 목표 / DoD
#68 던전은 타입별 단일 난이도뿐 — 고CP 고수에게 도전·선택 축이 없다. 티어로 CP 성장이 던전 보상에 직접 반영되는 무한 도전 추가.

### DoD 검증
1. **무한 티어**: 타입별 티어 1..N, `GetTierCpRequirement(type,tier)=minCp×2^(tier-1)`. `GetMaxAccessibleTier(type,cp)` CP 파생. 클라↔서버 `dungeon.ts` parity(Math.fround).
2. **티어 보상**: `GetDungeonReward(type,cp,tier)` = 기존 sqrt(CP/minCp) 보상 × `tierRewardMultiplier(tier)=max(1,tier)`. 티어 접근 불가(CP 미달) → 0.
3. **회귀 안전**: 티어1 = #68 기존 보상 동일(×1), 기존 2-인자 호출 기본 티어1.
4. **입장**: 타입별 일일 3회 공유(티어 무관 차감, #68 유지).
5. **세이브 변경 없음**(티어 CP 런타임 파생, SaveVersion 14 유지). #74 심연 마스터리 던전 보상 로컬 보너스는 티어 배수 후 단일 적용 유지.
6. **UI**: 던전 패널 티어 선택(해금/잠금·요구 CP·예상 보상) + ko/en.
7. **테스트**: 클라 Automation(게이트/보상 스케일/티어1 회귀/경계/입장 공유) + 서버 vitest + parity. UE Build/Automation + server-ci GREEN.

## 2. 범위 (In Scope)
2.1 공식(character+backend) — getTierCpRequirement/getMaxAccessibleTier/getDungeonReward(tier) 미러.
2.2 서비스/적용(character) — TryRunDungeon(Type,CP,Today,Tier) + GameInstance TryRunDungeon(Type,Tier).
2.3 UI(designer) — 티어 선택 + ko/en.
2.4 밸런스(balance) — 티어 CP/보상 곡선 + 입장 페이싱 문서.
2.5 테스트(qa) — DoD 7.

## 3. 범위 외 (후속)
- 티어별 고유 추가 보상(장비/소비 드롭), 주간 보스 던전, 티어별 입장 제한 분리, 최고 티어 도달 업적/연출.

## 4. 작업 분배 — Codex 호출
| 파트 | 작업 |
| --- | --- |
| character (메인) | 클라 던전 공식 tier, UDungeonService::TryRunDungeon(+Tier)·GetMaxAccessibleTier/GetTierCpRequirement, GameInstance TryRunDungeon(Type,Tier), 클라 Automation |
| backend | dungeon.ts tier 공식 미러 + vitest parity |
| designer | 던전 패널 티어 선택 UI + ko/en + CsvIntegrity |
| balance | TIER_CP_FACTOR/보상 곡선 + 입장 페이싱 + 무한 비폭주 |
| qa | 게이트/보상 스케일/티어1 회귀/입장 공유/경계 + parity |

## 5. 워크플로우 v3
[1] 기획+PR → [2] Codex 5-team → PM 게시 → [3] Claude TM 종합+fix → [4] Codex(결함 시) → [5] 검증(UE Automation 직접) → [N] CI 그린 + PM 종합 + 머지. PM 자율([[feedback-autonomous-slices]], [[feedback-ci-before-merge]]).

## 6. 리스크
| 리스크 | 완화 |
| --- | --- |
| 티어 보상 폭주 | 입장 3회/일 + 선형 보상 + balance |
| CP 게이트 과해금 | GetMaxAccessibleTier 경계 테스트(요구치±1) + parity |
| #68 회귀 | 티어1=기존 동일·기본 티어1·회귀 테스트 |
| #74 심연 마스터리 이중 | 티어 배수 후 심연 로컬 단일 적용(#72/#74 교훈) |
| 서버↔클라 drift | dungeon.ts Math.fround 미러 + 경계 parity |

## 7. 후속
티어별 고유 보상, 주간 보스 던전, 최고 티어 업적/연출.
