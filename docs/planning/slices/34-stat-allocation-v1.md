# PR #34 기획서 — 스탯 포인트 분배 (심화)

> 현재 캐릭터 1차 능력치는 `DefaultPrimaryStats(ClassId, Level)`로 **레벨에 따라 자동 결정**될 뿐, 플레이어가 빌드를 선택할 여지가 없다. 레벨업 시 **스탯 포인트**를 지급하고 STR/DEX/INT/WIS/CON/LUK 에 분배하게 해 캐릭터 성장 깊이/빌드 다양성을 더한다. 분배 포인트는 Primary 에 가산되어 기존 DeriveStats 를 통해 모든 2차 스탯에 자연 반영된다.

## 1. 목표 / DoD
레벨업 시 스탯 포인트를 얻고, 6개 1차 능력치에 분배하면 파생 능력치(공격력/HP/크리 등)가 즉시 상승한다.

### DoD 검증
1. 레벨업 시 스탯 포인트 N(StatPointFormula) 지급, 누적.
2. 분배: 가용 포인트>0 일 때 특정 1차 스탯에 +1(가용 -1, 분배 +1). 가용 0이면 분배 불가.
3. 분배분이 `DefaultPrimaryStats` 결과에 가산되어 RefreshDerivedStats → 파생 스탯/전투 스탯 즉시 반영.
4. 리셋: 분배 초기화(전 포인트 환원). 환생(레벨 리셋) 시 분배/가용 포인트도 리셋.
5. HUD 스탯 분배 패널(6스탯 현재값/가용 포인트/+버튼/리셋). 서버 StatPointFormula 미러 + parity. 서버 Vitest + UE 빌드/Automation GREEN. 기존 스탯/전투 회귀 없음.

## 2. 범위 (In Scope)
### 2.1 스탯 포인트 공식 (메인, C++)
- `StatPointFormula.h/.cpp`(CharacterSystem/, 순수 static): `GetStatPointsForLevelUp(int32 NewLevel)`(레벨업당 지급, 예 5 고정 또는 레벨 구간별), `GetTotalStatPointsForLevel(int32 Level)`(누적 — 검증/리셋용).
- `EPrimaryStat { Str, Dex, Int, Wis, Con, Luk }` 열거형(분배 타깃).
### 2.2 분배 상태/행위 (메인, C++)
- `UIdleGameInstance`: 가용 포인트 `AvailableStatPoints` + 분배 누적 `FPrimaryStats AllocatedStats`(또는 6 카운터). `GrantStatPoints(int32)`(레벨업 훅에서 호출), `AllocateStatPoint(EPrimaryStat)`(가용>0 시 +1), `ResetStatPoints()`(분배→가용 환원), `GetAllocatedPrimaryStats()→FPrimaryStats`, `GetAvailableStatPoints()`. 분배/리셋 시 델리게이트(OnStatPointsChanged) 브로드캐스트. 레벨업(HandleLevelUp/LevelUp 경로)에서 GrantStatPoints. 환생 Rebirth() 시 분배/가용 리셋.
- `AIdleCharacter::RefreshDerivedStats`: `Primary += GameInstance->GetAllocatedPrimaryStats()` 후 DeriveStats. 분배/리셋/레벨업 시 RefreshDerivedStats 재호출(OnStatPointsChanged 구독).
### 2.3 UI (디자이너)
- HUD 스탯 분배 패널: 6스탯 현재값(분배 포함) + 가용 포인트 + 스탯별 +버튼(HitBox) + 리셋 버튼. 가용 0 시 +버튼 비활성. 로컬라이즈 ko/en.
### 2.4 서버 (백엔드)
- `server/src/core/formulas/statPoints.ts`: getStatPointsForLevelUp/getTotalStatPointsForLevel 클라 미러 + (선택) applyAllocatedStats(primary, allocated) 헬퍼 + parity 테스트. 기존 stats.ts 와 일관.
### 2.5 데이터/밸런스
- 레벨당 포인트 수, 스탯→파생 영향(기존 DeriveStats 계수) 빌드 영향 + 문서.
### 2.6 테스트
- 서버 Vitest(공식/미러/parity) + 클라 Automation(레벨업 지급/누적, 분배 가산→파생 반영, 가용 0 가드, 리셋 환원, 환생 리셋).

## 3. 범위 외
- 스탯 리셋 골드 비용(V1 무료 리셋), 스탯 상한/소프트캡, 스탯 프리셋/저장(후속).
- 서버 권위 분배 정산(클라 권위 V1, 서버 공식 미러만).

## 4. 작업 분배 — Codex 호출
| 파트 | 작업 | 비중 |
| --- | --- | --- |
| 캐릭터·전투 (메인) | StatPointFormula + GameInstance 분배 상태/행위 + RefreshDerivedStats 통합 + 환생 리셋 + Automation | ✅ 메인 (`character`) |
| 백엔드 | statPoints.ts 미러 + parity | ✅ 보조 (`backend`) |
| 디자이너 | 스탯 분배 HUD 패널 + 로컬라이즈 | ✅ 보조 (`designer`) |
| 밸런스 | 레벨당 포인트/빌드 영향 + 문서 | ✅ 보조 (`balance`) |
| QA | 지급/분배/가드/리셋/환생 시나리오 | ✅ 보조 (`qa`) |

## 5. 워크플로우 v3
[1] 기획+PR → [2] Codex character 메인(+backend/designer/balance/qa) → [3] Claude TM → [4] Codex TM+fix → [5] 검증 → [N] **CI 그린 확정** + 머지. 사용자 PIE 차후([[feedback-autonomous-slices]]). 머지 전 CI 별도 확인([[feedback-ci-before-merge]]). **Codex 커밋/푸시 누락 PM 보완**([[reference-codex-invoke]]).

## 6. 리스크
| 리스크 | 완화 |
| --- | --- |
| 가용 포인트 음수/초과 분배 | AllocateStatPoint 에서 가용>0 선확인, 분배/가용 합 = 누적 일치 테스트 |
| 환생/레벨 리셋과 분배 정합 | Rebirth 시 분배·가용 리셋, GetTotalStatPointsForLevel 로 재지급 일관 |
| 분배가 파생 스탯에 미반영 | RefreshDerivedStats 에서 Primary 가산 + 구독 재계산, 파생 반영 Automation |
| 서버↔클라 공식 parity | StatPointFormula DefinitionParity 확장 |
| 기존 스탯/전투 회귀 | 분배 0 시 기존 DefaultPrimaryStats 결과 동일(가산 0) |

## 7. 후속
- 리셋 골드 비용, 스탯 소프트캡, 프리셋/저장, 서버 권위 분배 정산.
