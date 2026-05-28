# PR #68 기획서 — 던전 시스템 (엔드게임 반복 콘텐츠)

> **PM 자율 진행**(사용자 "PM 자율로 계속"). 직전 7 PR(룬/레어도/챕터3/유니크trait = 아이템·스탯 메타 심화) 후 **강해진 캐릭터로 도전하는 엔드게임 반복 콘텐츠**. 탑(#50 CP 무한 등반/영구 보너스)과 차별화: 던전 = **일일 제한 + 특화 재화 파밍**(골드/경험치/룬 에센스). CP(#49) 기반. client + server 멀티시스템(+designer/balance). [[project-infinite-growth]] 재화 수급 루프.

## 1. 목표 / DoD
3종 일일 던전(골드/경험치/룬 에센스 특화)을 일일 제한 내에서 CP 기반으로 클리어해 특화 재화를 파밍한다. 기존 CP(#49)·일일 리셋(#56 퀘스트)·재화(골드/경험치/룬 에센스 #61)·저장과 정합.

### DoD 검증
1. **데이터 모델**: `EDungeonType` 3종 — `Gold`/`Exp`/`Essence`. 각 던전 일일 입장 제한(예 3회/일, UTC 리셋). `FDungeonFormula` 보상/요구 CP 공식.
2. **공식**: `FDungeonFormula::GetRewardForCp(Type, CombatPower)` — CP 비례 특화 재화 보상(Gold던전→골드/Exp던전→경험치/Essence던전→룬 에센스). `GetDailyEntryLimit(Type)`(일일 횟수). `GetMinimumCp(Type)`(입장 최소 CP, 0 가능). 클라/서버 미러.
3. **서비스**: `UDungeonService` — 던전별 일일 입장 카운트 + `DailyResetDate`(UTC). `TryRunDungeon(Type, CombatPower)` → 입장 가능(횟수 남음)+CP 충족 시 보상 산출 + 입장 1회 차감, 보상 반환(GameInstance가 골드/경험치/에센스 지급). `GetRemainingEntries(Type)`. `CaptureState/RestoreState`(일일 리셋 #56 패턴).
4. **재화 지급 연동**: GameInstance가 `TryRunDungeon` 결과로 Gold→AddGold / Exp→AddExp / Essence→RuneEssence 지급(기존 경로). 3중 가드(입장 가능 1회 차감).
5. **저장(SaveVersion 9→10)**: 던전 일일 입장 카운트 + DailyResetDate. v<10 회귀안전(빈 카운트/리셋). 클라우드(#54) 자동 정합.
6. **서버 미러**: `server/src/core/formulas/dungeon.ts` + `dungeon.test.ts`(보상/요구 CP/일일 제한 parity, `Math.fround`).
7. **HUD**: 던전 패널(3종 + 잔여 입장/보상 미리보기/입장 버튼) + 로컬라이즈 ko/en.
8. **테스트**: 클라 Automation(입장 제한·일일 리셋·CP 게이트·보상 산출·재화 지급·저장 라운드트립/마이그레이션) + 서버 vitest(dungeon parity). UE Build/Automation + 서버 build/test/lint **GREEN**, server-ci 그린.

## 2. 범위 (In Scope)
### 2.1 던전 데이터/공식 (character + backend 미러)
EDungeonType + FDungeonFormula(보상/요구CP/일일제한) + 서버 dungeon.ts.
### 2.2 던전 서비스 (character)
UDungeonService(일일 입장/리셋/TryRunDungeon) + GameInstance 연동(재화 지급, 3중 가드).
### 2.3 저장 v10 (character)
입장 카운트/리셋 + 마이그레이션 회귀안전.
### 2.4 서버 미러 (backend)
dungeon.ts + parity test.
### 2.5 UI (designer)
던전 패널 HUD + 로컬라이즈 ko/en.
### 2.6 밸런스 (balance)
던전별 보상/CP 곡선 + 일일 제한 → 재화 수급 페이싱(기존 경제 영향) 시뮬.
### 2.7 테스트 — DoD 8.

## 3. 범위 외 (후속)
- 던전 난이도 티어 선택 · 던전 보스/웨이브 실제 전투 · 입장권 재화 · 주간 던전 · 던전 전용 드롭(유니크/룬).

## 4. 작업 분배 — Codex 호출
| 파트 | 작업 | 비중 |
| --- | --- | --- |
| 캐릭터·전투 (메인) | EDungeonType/FDungeonFormula + UDungeonService(일일 입장/리셋/CP 게이트) + GameInstance 재화 지급 + 저장 v10 + 서버 dungeon.ts + Automation | ✅ 메인 (`character`) |
| 디자이너 | 던전 패널 HUD(3종/잔여/보상) + 로컬라이즈 ko/en | ✅ 보조 (`designer`) |
| 밸런스 | 던전 보상/CP/일일제한 → 재화 수급 페이싱 시뮬 + 경제 영향 가드 | ✅ 보조 (`balance`) |
| (backend/qa) | character 흡수, [3] Claude TM parity·커버리지 | — |

## 5. 워크플로우 v3
[1] 기획+PR → [2] Codex character 메인(+designer/balance) → [3] Claude TM → [4] fix(필요시) → [5] 검증 → [N] **CI 그린 확정** + PM 종합 소견 + 머지. PM 자율([[feedback-autonomous-slices]]).

## 6. 리스크
| 리스크 | 완화 |
| --- | --- |
| 던전 보상이 경제 페이싱 교란(골드/에센스 과다) | balance 시뮬(일일 제한 × 보상 = 수급량 가드, 기존 median 영향) |
| 일일 리셋 경계(UTC) 오류 | #56 퀘스트 일일 리셋 패턴 재사용 + Automation 날짜 경계 |
| 입장 제한 우회(중복 차감 누락) | TryRunDungeon 3중 가드 1회 차감 + Automation |
| 탑(#50)과 중복 인식 | 던전=일일/특화 재화 파밍, 탑=무한 등반/영구 스탯 — 보상 성격 분리 명시 |
| 클라/서버 보상 불일치 | dungeon.ts parity(Math.fround) + 앵커 |
| 저장 v9→v10 회귀 | 빈 카운트/리셋 기본값 + 라운드트립/마이그레이션 Automation |
| 클라우드(#54) 정합 | UStructToJson 자동 포함(던전 필드) |

## 7. 후속
- 난이도 티어, 던전 보스 전투, 입장권, 주간 던전, 던전 전용 드롭.
