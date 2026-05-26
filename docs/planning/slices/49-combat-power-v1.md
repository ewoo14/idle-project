# PR #49 기획서 — 전투력 Combat Power 지표 (무한 성장 + 통합 검증)

> 무한 성장 시스템(강화50 #44 / Mythic #45 / 환생 scale #46 / 초월 #47 / 스킬50 #48 + 스탯분배 #34 / 장비 #36 / affix #40 / 세트 #43)이 모두 **파생 스탯(FDerivedStats)**으로 합성된다. 방치형 핵심 성장 체감 지표인 **전투력(Combat Power, CP)** — 파생 스탯을 단일 무한 증가 수치로 집계 — 를 추가해 HUD 에 표시한다. 동시에 CP 집계가 **전 성장 소스의 합성을 검증**하는 통합 테스트 역할을 한다. client+server+UI+balance+qa 5-team.

## 1. 목표 / DoD
플레이어의 현재 능력치가 단일 전투력(CP) 수치로 집계되어 HUD 에 표시되고, 모든 성장(강화/등급/세트/affix/스탯/환생/초월/스킬랭크)이 CP 를 증가시킨다.

### DoD 검증
1. CombatPowerFormula: FDerivedStats(+장비/세트/환생/초월 반영된 최종 Derived) → 단일 CP 정수(가중 합). 무한 증가(상한 없음).
2. CP 가 각 성장 소스 적용 시 증가(장비 강화↑·세트 충족·스탯 분배·환생 포인트·초월 배수·스킬 랭크가 데미지 경유 등 → 파생 스탯↑ → CP↑).
3. HUD 헤더(또는 정보 패널 #41)에 CP 표시.
4. 서버 CombatPowerFormula 미러(동일 파생 입력 → 동일 CP) + parity. 서버 Vitest + UE 빌드/Automation GREEN.
5. **통합 검증**: 완전 성장 캐릭터(분배+장비[affix+세트]+환생+초월) 의 파생 스탯·CP 합성 정합 Automation. 기존 전투/스탯 회귀 없음(CP 는 읽기/표시).

## 2. 범위 (In Scope)
### 2.1 전투력 공식 (메인, C++)
- `CombatPowerFormula.h/.cpp`(CharacterSystem 또는 GameCore, 순수 static): `ComputeCombatPower(const FDerivedStats&)` → int64. 가중 합 예: PhysAtk×1 + MagicAtk×1 + PhysDef×2 + MagicDef×2 + Hp×0.1 + CritRate×500 + CritDmg×100 + AtkSpeed×200 (round, 최소 0). 무한.
### 2.2 노출/통합 (메인, C++)
- `AIdleCharacter::GetCombatPower()`(BlueprintPure) → ComputeCombatPower(GetCurrentDerivedStats())(#41 캐싱 Derived = 최종 합성). RefreshDerivedStats 후 최신.
### 2.3 UI (디자이너)
- HUD 헤더(골드/EXP/레벨 근처) 또는 정보 패널(#41)에 "전투력 N" 표시. 큰 수 천단위 콤마. 로컬라이즈 ko/en.
### 2.4 서버 (백엔드)
- `server/src/core/formulas/combatPower.ts`: computeCombatPower(derivedStats) 클라 미러(Math.fround 가중) + parity 테스트.
### 2.5 데이터/밸런스
- CP 가중치 근거 + 성장 단계별 CP 예시(초기/중반/엔드) + 무한 증가 맥락 문서.
### 2.6 테스트 (통합 검증 핵심)
- 서버 Vitest(CP 공식/미러/parity) + 클라 Automation:
  - ComputeCombatPower(파생 스탯 → CP, 0 가드).
  - **통합**: 분배+장비(affix+세트)+환생포인트+초월배수 적용 캐릭터의 GetCurrentDerivedStats → CP 가 각 소스 증가에 따라 단조 증가(강화↑/세트충족/스탯분배/환생/초월 → CP↑).
  - GetCombatPower = ComputeCombatPower(GetCurrentDerivedStats) 일치.

## 3. 범위 외
- CP 기반 매칭/랭킹/콘텐츠 게이팅(후속), CP 변화 연출/그래프, 스킬 랭크의 CP 직접 가산(현재 스킬은 데미지 경유 — CP 는 파생 스탯 기반).
- 서버 권위 CP(클라 표시 V1, 서버 공식 미러만).

## 4. 작업 분배 — Codex 호출
| 파트 | 작업 | 비중 |
| --- | --- | --- |
| 캐릭터·전투 (메인) | CombatPowerFormula + GetCombatPower + 통합 Automation | ✅ 메인 (`character`) |
| 백엔드 | combatPower.ts 미러 + parity | ✅ 보조 (`backend`) |
| 디자이너 | CP HUD 표시 + 로컬라이즈 | ✅ 보조 (`designer`) |
| 밸런스 | CP 가중치/성장 예시 + 문서 | ✅ 보조 (`balance`) |
| QA | CP 증가/통합 합성/E2E 성장 루프 시나리오 | ✅ 보조 (`qa`) |

## 5. 워크플로우 v3
[1] 기획+PR → [2] Codex character 메인(+backend/designer/balance/qa) → [3] Claude TM → [4] Codex TM+fix → [5] 검증 → [N] **CI 그린 확정** + 머지. 사용자 PIE 차후([[feedback-autonomous-slices]]). 머지 전 CI 별도 확인([[feedback-ci-before-merge]], [[reference-ci-retrigger]]). **Codex 커밋/푸시 누락 PM 보완**([[reference-codex-invoke]]).

## 6. 리스크
| 리스크 | 완화 |
| --- | --- |
| CP 가중치로 일부 스탯 과대/과소 대표 | 보수적 가중 + balance 문서, V1 단순 가중(후속 튜닝) |
| 큰 수 오버플로 | int64 CP, round/clamp |
| 서버↔클라 CP parity | combatPower.ts Math.fround 가중 + 경계 테스트 |
| 통합 합성 누락(소스 미반영) | GetCurrentDerivedStats(#41, 최종 합성 캐싱) 사용 → 전 소스 자동 포함, 통합 Automation 으로 단조 증가 검증 |
| 기존 전투/스탯 회귀 | CP 는 읽기/표시 전용, 공식/전투 무변경 |

## 7. 후속
- CP 기반 콘텐츠 게이팅/랭킹, CP 변화 연출, 스킬/펫의 CP 가산 정교화, 서버 권위 CP.
