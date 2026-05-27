# PR #53 기획서 — 저장/로드 Save System v2 (Inventory/Skill/Quest/Season)

> **#52 Save System V1 의 명시적 후속.** V1 은 GameInstance 코어(골드/레벨/환생/초월/스탯분배/보스)+Stage/Tower/Pet 만 저장하고 **Inventory(장비)·Skill 랭크·Quest 진행·Season 토큰/티어는 범위 외**로 남겨, 재시작 시 이 네 시스템이 전부 초기화된다. 강화(#33·#39·#44)·드롭(#36·#45)·affix(#40)·세트(#43)·스킬 무한랭크(#48)·퀘스트(#18)·시즌(#22)의 누적 진행이 영속되도록 SaveGame 스키마를 v2 로 확장하고 각 시스템에 capture/restore 경로를 추가한다. client 인프라 슬라이스(+qa/designer).

## 1. 목표 / DoD
게임 종료 후 다시 켜도 **인벤토리(보유 장비·장착·강화·affix·세트)·스킬 랭크/포인트·퀘스트 진행/수령·시즌 토큰/수령 티어**가 그대로 복원된다. V1 코어 진행과 함께 단일 세이브로 저장된다.

### DoD 검증
1. `UIdleSaveGame` SaveVersion `1 → 2` 승격 + 신규 필드 추가:
   - **Inventory**: `TArray<FItemInstance> InventoryItems`(보유 장비 전체) + `TMap<EItemSlot,int32> EquippedSlotIndex`(장착 슬롯→Items 인덱스). FItemInstance 의 ItemId/Slot/Rarity/ItemSet/보너스(Atk/Def/Hp/CritRate/AtkSpeed/MagicAtk)/EnhanceLevel/DisplayName 무손실.
   - **Skill**: `TMap<FName,int32> SkillRanks` + `int32 SkillPoints`(게이지/쿨다운/버프 등 휘발성 런타임은 제외).
   - **Quest**: `TArray<FQuestSaveEntry> Quests`(QuestId/Progress/bCompleted/bClaimed/Type/DailyResetDate) + `FString QuestDailyResetDate`.
   - **Season**: `int32 SeasonId` + `int32 SeasonTokens` + `TArray<int32> SeasonClaimedTiers`.
2. **capture/apply 확장**: `UIdleGameInstance::CaptureToSave` 가 V1 코어에 더해 위 4시스템 상태 수집, `ApplyFromSave` 가 복원. 인벤/스킬은 `FindPlayerCharacter()`→`UInventoryComponent`/`USkillComponent`, 퀘스트/시즌은 `EnsureQuestService()`/`EnsureSeasonService()` 경유.
3. **restore API 추가**(기존 set/init 경로 재사용·확장):
   - `UInventoryComponent::CaptureState(OutItems, OutEquipped)` / `RestoreState(Items, Equipped)` — 인덱스 유효성/슬롯 정합 클램프.
   - `USkillComponent::CaptureRankState(OutRanks, OutPoints)` / `RestoreRankState(Ranks, Points)` — MaxRank(50) 클램프, 미정의 SkillId 무시.
   - `UQuestService::CaptureState` / `RestoreState` — 정의 존재 퀘스트만, Progress≥0·TargetCount 클램프, 로드 후 `ResetDailyQuestsIfNeeded(today)` 재적용(날짜 경과 시 일일 리셋).
   - `USeasonService::CaptureState` / `RestoreState` — SeasonId 불일치 시 토큰/티어 무시(시즌 종료 안전), 토큰≥0.
4. **v1 → v2 마이그레이션**: SaveVersion==1 세이브 로드 시 신규 4시스템 필드는 기본값(빈 인벤/스킬0/퀘스트 기본 init/시즌0) → **회귀안전**. 저장 시 항상 SaveVersion=2.
5. **autosave 연동**: 장비 획득/장착/강화·스킬 랭크업·퀘스트 수령·시즌 수령 후 기존 `RequestAutosave()`(1.0s 디바운스, #52) 트리거.
6. 라운드트립 Automation: 인벤(여러 아이템+장착+강화+affix+set)·스킬랭크/포인트·퀘스트(진행/완료/수령/일일)·시즌(토큰/수령티어) 전수 무손실 + v1→v2 마이그레이션 no-op + malformed(잘못된 인덱스/미정의 ID/시즌 불일치) 새니타이즈. UE 빌드/Automation GREEN.

## 2. 범위 (In Scope)
### 2.1 SaveGame 스키마 v2 (메인, C++)
- `UIdleSaveGame`: 위 Inventory/Skill/Quest/Season 필드 + SaveVersion 2. 직렬화 보조 USTRUCT(`FQuestSaveEntry`) 필요 시 추가. FText DisplayName 은 USTRUCT 직접 직렬화(또는 ItemId/Rarity/Slot 기반 재구성 — Codex 판단, 단 라운드트립 보장).
### 2.2 capture/apply + restore API (메인, C++)
- `UIdleGameInstance::CaptureToSave`/`ApplyFromSave` 확장(autosave 억제 가드 #52 유지). 컴포넌트/서비스 접근 + restore 호출.
- 위 4개 시스템 CaptureState/RestoreState API. 컴포넌트(Inventory/Skill)는 캐릭터 부재 시 graceful skip.
### 2.3 UI (디자이너, 최소)
- 기존 "저장됨" 토스트(#52 OnProgressSaved) 재사용 — 추가 HUD 없음 또는 인벤/스킬 복원 후 갱신 보장(델리게이트 재방송). 신규 로컬라이즈 키 있으면 ko/en.
### 2.4 테스트 (핵심)
- 클라 Automation 라운드트립/마이그레이션/새니타이즈(위 DoD 6).

## 3. 범위 외 (save-v3+ 후속)
- 서버 클라우드 세이브/동기화, 다중 세이브 슬롯, 저장 암호화/무결성 해시, 세이브 가져오기/내보내기.
- 시즌 교체(SeasonId 증가) 시 보상 마이그레이션 정책(현재는 불일치 무시).

## 4. 작업 분배 — Codex 호출
| 파트 | 작업 | 비중 |
| --- | --- | --- |
| 캐릭터·전투 (메인) | SaveGame v2 스키마 + GameInstance capture/apply 확장 + Inventory/Skill/Quest/Season CaptureState/RestoreState + 마이그레이션 + Automation | ✅ 메인 (`character`) |
| 디자이너 | 복원 후 HUD 갱신/토스트 정합 + (필요 시) 로컬라이즈 | ✅ 보조 (`designer`) |
| QA | 인벤/스킬/퀘스트/시즌 저장→재시작 복원 + v1 세이브 마이그레이션 시나리오 | ✅ 보조 (`qa`) |

## 5. 워크플로우 v3
[1] 기획+PR → [2] Codex character 메인(+designer/qa) → [3] Claude TM → [4] Codex TM+fix → [5] 검증 → [N] **CI 그린 확정** + 머지. 사용자 PIE 차후([[feedback-autonomous-slices]]). 머지 전 CI 별도 확인([[feedback-ci-before-merge]], [[reference-ci-retrigger]]). Codex 커밋/푸시 누락 PM 보완([[reference-codex-invoke]]).

## 6. 리스크
| 리스크 | 완화 |
| --- | --- |
| FItemInstance(FText 포함) 직렬화 손실 | USTRUCT 직접 직렬화 라운드트립 Automation 으로 전 필드 비교, 손실 시 ItemId 기반 재구성 |
| 장착 인덱스(EquippedSlotIndex)가 Items 재배열 후 어긋남 | capture/restore 를 Items+Equipped 동시 처리, restore 시 인덱스 범위·슬롯 일치 검증 |
| 퀘스트 일일 리셋이 로드 시점에 안 돌아감 | RestoreState 후 `ResetDailyQuestsIfNeeded(today)` 강제 재적용 |
| 시즌 교체 후 옛 토큰/티어 복원 | SeasonId 불일치 시 무시(기본 신규 시즌) |
| v1 세이브 호환 깨짐 | SaveVersion 분기, v1 로드 시 신규 필드 기본값(빈/0/기본 init) = 회귀안전, Automation 마이그레이션 케이스 |
| 기존 코어(#52) 저장 회귀 | V1 필드 capture/apply 무변경, 라운드트립 Automation 코어 케이스 유지 |

## 7. 후속 (save-v3+)
- 서버 클라우드 세이브/동기화, 다중 슬롯, 무결성 해시, 세이브 import/export, 시즌 교체 보상 정책.
