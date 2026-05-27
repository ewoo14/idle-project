# PR #52 기획서 — 게임 저장/로드 Save System (핵심 인프라)

> **방치형 게임의 필수 인프라.** 현재 핵심 진행(골드/레벨/EXP/환생#46/초월#47/스탯분배#34/탑#50·#51/펫#42)이 **저장되지 않아 재시작 시 전부 소실**된다(Language·LastSeenUnixSec 만 GConfig 저장). 무한 성장(#44~#51)이 의미를 가지려면 **진행 영속**이 필수. 로컬 USaveGame 으로 핵심 상태를 저장/로드한다. client 인프라 슬라이스(+qa/designer).

## 1. 목표 / DoD
게임 종료 후 다시 켜도 골드·레벨·환생·초월·탑·펫 등 핵심 진행이 그대로 복원된다.

### DoD 검증
1. `UIdleSaveGame`(USaveGame)에 핵심 상태 저장: GameInstance(Gold/CharacterLevel/CurrentExp/NextExp/RebirthCount/RebirthBonusPoints/TranscendCount/AvailableStatPoints/AllocatedStats/bChapter1BossDefeated/LastSeenUnixSec) + Stage(Chapter/Stage/KillsThisStage/bFinalChapterCleared/HighestClearedChapter) + Tower(HighestFloor) + Pet(EquippedPetId/PetLevels).
2. `SaveProgress()`: 현재 상태 → UIdleSaveGame → SaveGameToSlot. `LoadProgress()`: LoadGameFromSlot → 각 필드/서비스 복원(restore API).
3. Init 시 LoadProgress(저장 있으면 복원, 없으면 기본). Shutdown 시 SaveProgress + 주기 autosave(예 골드/레벨 변경 또는 N초).
4. 저장 라운드트립 정합(저장 후 로드 = 동일 상태). 저장 없을 때 기본값(신규 게임). 서버 무관(클라 로컬). UE 빌드/Automation GREEN.

## 2. 범위 (In Scope)
### 2.1 SaveGame 데이터 (메인, C++)
- `UIdleSaveGame`(GameCore/IdleSaveGame.h, UCLASS : USaveGame): 위 핵심 필드(UPROPERTY). SaveVersion(int32, 마이그레이션 대비).
### 2.2 저장/로드 (메인, C++)
- `UIdleGameInstance`: `void SaveProgress()`(상태 수집→SaveGameToSlot("IdleSave",0)), `void LoadProgress()`(LoadGameFromSlot→GameInstance 필드 + StageService/TowerService/PetService restore). Init 에서 LoadProgress(서비스 Ensure 후), Shutdown 에서 SaveProgress. 주기/이벤트 autosave(AddGold/LevelUp/Rebirth/Transcend/ClimbTower/FeedPet 등 주요 변경 후 SaveProgress 호출 — 또는 디바운스).
- 서비스 restore API: UStageService::RestoreState(...), UTowerService::SetHighestFloor(...), UPetService::RestoreState(equipped, levels). (기존 상태 set 경로 없으면 추가.)
### 2.3 UI (디자이너)
- 저장 표시(헤더에 "저장됨" 토스트 또는 autosave 인디케이터, 최소). 로컬라이즈 ko/en.
### 2.4 테스트 (핵심)
- 클라 Automation: SaveProgress→LoadProgress 라운드트립(주입 GameInstance/서비스 상태 저장 후 복원 = 동일), 저장 없을 때 기본값, SaveVersion. (SaveGameToSlot 파일 IO 어려우면 UIdleSaveGame ↔ 상태 직렬화/역직렬화 순수 로직 분리해 테스트.)

## 3. 범위 외 (save-v2 후속)
- Inventory(장비 TArray) / Skill 랭크 / Quest / Season 상태 저장(후속 save-v2 — restore API 추가 필요).
- 서버 클라우드 세이브/동기화(V1 로컬), 다중 슬롯, 저장 암호화/무결성.

## 4. 작업 분배 — Codex 호출
| 파트 | 작업 | 비중 |
| --- | --- | --- |
| 캐릭터·전투 (메인) | UIdleSaveGame + GameInstance SaveProgress/LoadProgress + autosave + 서비스 restore + Automation | ✅ 메인 (`character`) |
| 디자이너 | 저장 인디케이터/토스트 + 로컬라이즈 | ✅ 보조 (`designer`) |
| QA | 저장/로드 라운드트립·신규·재시작 시나리오 | ✅ 보조 (`qa`) |

## 5. 워크플로우 v3
[1] 기획+PR → [2] Codex character 메인(+designer/qa) → [3] Claude TM → [4] Codex TM+fix → [5] 검증 → [N] **CI 그린 확정** + 머지. 사용자 PIE 차후([[feedback-autonomous-slices]]). 머지 전 CI 별도 확인([[feedback-ci-before-merge]], [[reference-ci-retrigger]]). **Codex 커밋/푸시 누락 PM 보완**([[reference-codex-invoke]]).

## 6. 리스크
| 리스크 | 완화 |
| --- | --- |
| 저장/로드 상태 누락·불일치 | 라운드트립 Automation(저장 후 로드=동일), 필드 전수 + SaveVersion |
| 서비스 복원 경로 부재 | StageService/TowerService/PetService restore API 추가(기존 set/init 재사용) |
| Init 로드 타이밍(서비스 Ensure 전) | LoadProgress 를 서비스 Ensure 후 호출, 순서 명확 |
| autosave 과다 IO | 주요 이벤트 후 저장 + (선택) 디바운스, Shutdown 저장 보장 |
| 기존 동작 회귀 | 저장 없을 때 기존 기본 초기화 유지, 로드는 있을 때만 덮어씀 |

## 7. 후속 (save-v2)
- Inventory/Skill/Quest/Season 저장, 클라우드 세이브/서버 동기화, 다중 슬롯, 무결성/마이그레이션.
