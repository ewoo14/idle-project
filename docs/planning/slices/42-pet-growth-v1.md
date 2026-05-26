# PR #42 기획서 — 펫 성장 (레벨업 + 골드 먹이기) (심화)

> 펫(#22)은 고정 보너스(강아지 골드+20%, 새 드롭+15%)뿐이라 성장 요소가 없다. **골드로 펫을 먹여 레벨업**하면 보너스 배수가 오르게 해 펫에 진행 깊이를 더하고, 동시에 **골드 싱크**(엔드게임 골드 적체 #38/#39 일부 완화)를 추가한다. client+server+UI+balance+qa 전반을 다루는 충실한 멀티시스템 슬라이스([[feedback-substantial-slices]]).

## 1. 목표 / DoD
플레이어가 골드를 소모해 펫을 레벨업하면, 펫 보너스(골드/드롭)가 레벨에 비례해 커진다.

### DoD 검증
1. 펫 레벨: 펫별 레벨(0/1~MaxPetLevel). 먹이기 시 골드 비용(레벨 비례) 차감 → 레벨 +1(최대치 가드). 골드 부족/최대치 시 불가(차감 없음).
2. 보너스 스케일: 장착 펫 유효 보너스 = bonusPercent × GetBonusMultiplier(level). ApplyGoldBonus/ApplyDropBonusChance 가 레벨 반영 보너스 사용.
3. 먹이기 시 결과(레벨/비용) 반환 + 델리게이트(HUD/연출).
4. HUD 펫 패널에 펫별 레벨/다음 비용/먹이기 버튼 + 레벨반영 보너스 표시.
5. 서버 PetLevelFormula 미러(피드 비용/보너스 배수) + parity. 서버 Vitest + UE 빌드/Automation GREEN. 기존 펫 장착/보너스(레벨 0 = 기존) 회귀 없음.

## 2. 범위 (In Scope)
### 2.1 펫 레벨 공식 (메인, C++)
- `PetLevelFormula.h/.cpp`(GameCore/, 순수 static): `MaxPetLevel`(예 10), `GetFeedCost(int32 CurrentLevel)`(레벨↑ 비용↑, 골드 싱크 곡선), `GetBonusMultiplier(int32 Level)`(예 1 + level×0.1 → Lv10 ×2).
### 2.2 펫 성장 상태/행위 (메인, C++)
- `UPetService`: 펫별 레벨 `TMap<FString,int32> PetLevels`. `FeedPet(PetId)`(레벨 +1, 최대치 가드, 성공 시만 — 골드는 GameInstance 책임). `GetPetLevel(PetId)`. 유효 보너스 게터(GetEquippedPetGoldBonusPercent/DropBonusPercent)가 ×GetBonusMultiplier(level) 적용. ApplyGoldBonus/ApplyDropBonusChance 가 레벨 반영.
- `UIdleGameInstance::TryFeedPet(const FString& PetId)` → `FPetFeedResult{bFed,GoldSpent,NewLevel}`: 펫 존재/레벨<Max 확인 → 비용=GetFeedCost(level) → GetGold()≥비용 확인 → AddGold(-비용) 1회 → PetService->FeedPet → Record... 결과+델리게이트(OnPetFed). 골드 부족/최대치 시 차감 없음.
### 2.3 UI (디자이너)
- HUD 펫 패널(기존 DrawPetPanel 확장): 펫별 레벨/다음 비용/먹이기 버튼(HitBox 예 "PetFeed_") + 레벨반영 보너스(%) 표시. 골드 부족/최대치 비활성. 결과 피드백. 로컬라이즈 ko/en.
### 2.4 서버 (백엔드)
- `server/src/core/formulas/petLevel.ts`(또는 pets.ts 확장): getFeedCost/getBonusMultiplier/MAX_PET_LEVEL 클라 미러 + parity 테스트. (pet 모듈 영속은 V1 클라 권위, 서버 공식 미러.)
### 2.5 데이터/밸런스
- 피드 비용 곡선(골드 싱크 규모 — #32 유입 대비)·보너스 배수 + 문서. 골드펫 레벨업이 골드 보너스↑ 피드백 고려(피드 비용이 순 싱크로 우세한지 점검).
### 2.6 테스트
- 서버 Vitest(피드 비용/보너스 배수/미러/parity) + 클라 Automation(레벨업 골드 게이트, 최대치 가드, 레벨반영 보너스, ApplyGoldBonus/Drop 레벨 반영, 레벨 0 회귀).

## 3. 범위 외
- 펫 합성/진화/신규 펫 추가(후속), 펫 스킬/액티브, 펫 외형/연출(외부).
- 서버 권위 펫 레벨 영속(클라 권위 V1, 서버 공식 미러).

## 4. 작업 분배 — Codex 호출
| 파트 | 작업 | 비중 |
| --- | --- | --- |
| 캐릭터·전투 (메인) | PetLevelFormula + PetService 레벨/FeedPet + GameInstance.TryFeedPet + 레벨반영 보너스 + Automation | ✅ 메인 (`character`) |
| 백엔드 | petLevel.ts 미러 + parity | ✅ 보조 (`backend`) |
| 디자이너 | 펫 패널 레벨/먹이기 HUD + 로컬라이즈 | ✅ 보조 (`designer`) |
| 밸런스 | 피드 비용/보너스 배수 + 골드 싱크 분석 + 문서 | ✅ 보조 (`balance`) |
| QA | 레벨업/골드부족/최대치/보너스 반영 시나리오 | ✅ 보조 (`qa`) |

## 5. 워크플로우 v3
[1] 기획+PR → [2] Codex character 메인(+backend/designer/balance/qa) → [3] Claude TM → [4] Codex TM+fix → [5] 검증 → [N] **CI 그린 확정** + 머지. 사용자 PIE 차후([[feedback-autonomous-slices]]). 머지 전 CI 별도 확인([[feedback-ci-before-merge]], [[reference-ci-retrigger]]). **Codex 커밋/푸시 누락 PM 보완**([[reference-codex-invoke]]).

## 6. 리스크
| 리스크 | 완화 |
| --- | --- |
| 골드 음수/이중 차감/최대치 | TryFeedPet 보유·레벨 선확인 후 1회 차감, 시도/성공 분리 테스트 |
| 골드펫 레벨업이 골드 보너스↑ 피드백(인플레) | 피드 비용을 순 싱크로 우세하게(balance 점검), 보수적 배수 |
| 서버↔클라 공식 parity | PetLevelFormula DefinitionParity 확장(정수/float 라운딩 주의) |
| 기존 펫 장착/보너스 회귀 | 레벨 0 = GetBonusMultiplier 1.0 → 기존 보너스 동일 |

## 7. 후속
- 펫 합성/진화/신규 펫, 펫 스킬, 서버 권위 펫 레벨 영속, 골드 유입 정밀 튜닝.
