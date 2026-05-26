# PR #32 기획서 — 처치 보상 스케일링 (심화)

> PR #31 스테이지 진행으로 몬스터는 스테이지마다 강해지지만(HP/Atk ×배수), **처치 보상(EXP·골드)은 여전히 고정**(EXP 12 / 골드 10+rand / 드롭 품질 monster level 1). 고스테이지가 어렵기만 하고 보상이 같아 진행이 공허하다. 처치 보상을 스테이지/보스에 비례시켜 방치 진행 루프를 완성한다.

## 1. 목표 / DoD
몬스터 처치 보상(EXP·골드·드롭 품질)이 해당 몬스터가 속한 스테이지가 높을수록, 보스일수록 커진다.

### DoD 검증
1. 처치 EXP/골드가 `RewardMultiplier(GlobalStageIndex)`(스테이지 진행도)에 비례 + 보스는 추가 보너스(예 ×N).
2. 드롭 아이템 품질(`RandomDropFromMonster(level)`)이 스테이지에 따라 상향(스테이지→몬스터 레벨 매핑).
3. 보상은 **스폰 시점 스테이지** 기준(스폰~처치 사이 스테이지 진행과 무관, 결정적).
4. 서버 RewardFormula 미러 + parity. 기존 펫 골드 보너스/드롭 확률(#22) 연동 유지. 서버 Vitest + UE 빌드/Automation GREEN. 기존 전투/드롭 회귀 없음.

## 2. 범위 (In Scope)
### 2.1 보상 공식 (메인, C++)
- `RewardFormula.h/.cpp`(GameCore 또는 CombatSystem, 순수 static):
  - `ComputeKillExp(int64 BaseExp, int32 GlobalStageIndex, bool bIsBoss)` = BaseExp × RewardMultiplier × (보스? BossBonus : 1)
  - `ComputeKillGold(int64 BaseGold, int32 GlobalStageIndex, bool bIsBoss)` 동형
  - `GetMonsterLevelForStage(int32 GlobalStageIndex)`(드롭 품질용 몬스터 레벨 매핑, 예 1 + idx)
  - RewardMultiplier 는 `FStageFormula::ComputeRewardMultiplier`(=1+idx×0.15) 재사용. BossBonus 상수(예 8).
### 2.2 몬스터 처치 적용 (메인, C++)
- `AIdleMonster`: 스폰 시 `StageGlobalIndex` 보관(GameMode 가 SetStageStatMultiplier 처럼 SetStageContext/SetStageGlobalIndex 설정). HandleDeath 에서 RewardFormula 로 EXP/골드/드롭레벨 계산. 보스 여부는 기존 bIsBoss. 펫 골드 보너스(#22)는 스케일 골드에 곱(기존 ApplyEquippedPetGoldBonus 순서 유지).
- `GameMode::SpawnMonsterAt`: 스테이지 인덱스도 몬스터에 전달.
### 2.3 서버 (백엔드)
- `server/src/core/formulas/reward.ts`: computeKillExp/computeKillGold/getMonsterLevelForStage 클라 미러 + BossBonus 상수 일치 + parity 테스트.
### 2.4 데이터/밸런스
- BossBonus, 드롭 레벨 매핑, 보상 곡선 + 문서. `tools/balance-sim` 에 보상 스케일 반영(보상 vs 몬스터 HP 스케일 → 진행 시간 곡선 점검, 환생 도달 타깃 유지 확인).
### 2.5 테스트
- 서버 Vitest(공식/미러/parity) + 클라 Automation(EXP/골드/드롭레벨 스케일·보스 보너스·스폰시점 기준).

## 3. 범위 외
- 골드/EXP 외 재화(시즌토큰 등) 스케일, 드롭 테이블 스테이지별 차등 풀(후속), 보상 팝업 UI 변경(기존 골드 픽업 유지).
- 서버 권위 보상 정산(클라 권위 V1, 서버 공식 미러만).

## 4. 작업 분배 — Codex 호출
| 파트 | 작업 | 비중 |
| --- | --- | --- |
| 캐릭터·전투 (메인) | RewardFormula + IdleMonster/GameMode 적용 + Automation | ✅ 메인 (`character`) |
| 백엔드 | reward.ts 미러 + parity | ✅ 보조 (`backend`) |
| 밸런스 | BossBonus/드롭레벨/보상곡선 + balance-sim 반영 + 문서 | ✅ 보조 (`balance`) |
| QA | EXP/골드/드롭 스케일·보스 보너스 시나리오 | ✅ 보조 (`qa`) |

## 5. 워크플로우 v3
[1] 기획+PR → [2] Codex character 메인(+backend/balance/qa) → [3] Claude TM → [4] Codex TM+fix → [5] 검증 → [N] **CI 그린 확정** + 머지. 사용자 PIE 차후([[feedback-autonomous-slices]]). 머지 전 CI 별도 확인([[feedback-ci-before-merge]]). **Codex 커밋/푸시 누락 PM 보완**([[reference-codex-invoke]]).

## 6. 리스크
| 리스크 | 완화 |
| --- | --- |
| 보상-난이도 곡선 불균형(보상 폭주/부족) | RewardMultiplier=HP배수와 동일 계수 재사용 + balance-sim 진행시간 점검, 보수적 BossBonus |
| 스폰~처치 사이 스테이지 변화로 보상 흔들림 | 스폰 시점 GlobalStageIndex 보관(결정적) |
| 펫 골드 보너스 순서 회귀 | 스케일 후 ApplyEquippedPetGoldBonus 곱(기존 호출 보존) + 테스트 |
| 서버↔클라 공식 parity | RewardFormula DefinitionParity 확장 |
| 기존 EXP/드롭 회귀 | 미초기화/idx 0 시 BaseExp×1.0 = 기존 값 보존 |

## 7. 후속
- 드롭 테이블 스테이지별 차등 풀, 보상 팝업/연출, 서버 권위 보상 정산, 챕터2+ 보상 곡선.

## 8. Codex character result
- Added `FRewardFormula` in `client/Source/IdleProject/GameCore/` with kill EXP, kill gold, and monster-level scaling.
- `AIdleMonster` stores spawn-time `StageGlobalIndex`; death rewards now use the stored index and boss bonus while keeping pet gold/drop bonus order.
- `AIdleProjectGameModeBase::SpawnMonsterAt` passes `StageInfo.GlobalStageIndex` into spawned monsters.
- Automation added: `IdleProject.GameCore.RewardFormula.KillRewardScaling`, `IdleProject.GameCore.RewardFormula.MonsterLevelForStage`, `IdleProject.Combat.Monster.RewardStageContext`.
- Verification: `Build.bat IdleProjectEditor Win64 Development` succeeded; `Automation RunTests IdleProject` completed with exit code 0.
