# PR #65 기획서 — 레어도 7단계 재정의 (전 시스템 기반)

> **사용자 지시(2026-05-28)**: 레어도를 **일반 → 레어 → 에픽 → 유니크 → 전설 → 초월 → 신화** 7단계로 재정의. enum 영어 식별자, 표시명 ko 한글. **장비·룬 등 모든 아이템 적용(소비 아이템 제외)**. [[project-rarity-overhaul]]. 룬 트랙(#61~#64) 후 진행(사용자 명시). 50여 파일 + 모든 공식 + 저장 마이그레이션에 걸친 **대규모 횡단 변경**. client + server 멀티시스템(+designer/balance/qa).

## 1. 목표 / DoD
레어도가 7단계(Common/Rare/Epic/Unique/Legendary/Transcendent/Mythic)로 통일되고, 기존 6단계 세이브가 회귀안전하게 마이그레이션된다. 모든 공식(드롭/강화/세트/룬/도감/직업룬)·서버 미러·클라우드·HUD·로컬라이즈 정합.

### DoD 검증
1. **`EItemRarity` 재정의**: `None=0 / Common=1 / Rare=2 / Epic=3 / Unique=4 / Legendary=5 / Transcendent=6 / Mythic=7`. **Uncommon 제거**, **Unique/Transcendent 신규**. ko 표시명 일반/레어/에픽/유니크/전설/초월/신화.
2. **마이그레이션(이름 기준, Uncommon→Rare 승급)**: `FRarityMigration::MigrateLegacy(int32 oldValue)`:
   | 기존(6단계 값) | → 신규(7단계 값) |
   | --- | --- |
   | 1 Common | 1 Common |
   | 2 Uncommon | 2 Rare (승급) |
   | 3 Rare | 2 Rare |
   | 4 Epic | 3 Epic |
   | 5 Legendary | 5 Legendary |
   | 6 Mythic | 7 Mythic |
   - `ApplyFromSave`에서 `SaveVersion < 7`이면 `InventoryItems`·`Runes`(+`RuneCodex` 셀 키)의 Rarity 변환. **SaveVersion 6→7**.
3. **신규 등급 상수 보간**: 모든 레어도 비례 공식에 Unique(에픽<유니크<전설)·Transcendent(전설<초월<신화) 값을 기존 곡선 보간으로 추가, Uncommon 케이스 제거. 대상 공식(grep `EItemRarity`/`ItemRarity` 전수):
   - `DropFormula`(GetRarityStatMultiplier/RollRarityForLevel 곡선/색), `EnhanceFormula`(RarityCostMultiplier), `RuneFormula`(코어 base·step/유틸 base/분해/RollRuneRarity), `ClassRuneFormula`(craft cost), `SetBonusFormula`(장비 세트 RollItemSet), `RuneSetFormula`(RollRuneSet 게이트), `RuneCodexFormula`(행 보너스 7레어도)
4. **룬 도감(#62) 확장**: 셀 타입9 × **레어도7 = 63셀**(기존 54). `RuneCodexFormula::TotalCells` 54→63, 코어 35(5×7)/유틸 28(4×7), 행 보너스 7레어도(Unique/Transcendent 행 추가, balance 곡선). 도감 마이그레이션(기존 54셀 → 63셀, 신규 등급 행 미unlock + Uncommon 셀 키 → Rare 병합).
5. **서버 미러 + 클라우드**: 전 공식 `.ts`(drop/enhance/setBonus/rune/runeCodex/runeSet/classRune) ItemRarity 1~6 → 1~7 + 상수 보간 + Uncommon 제거. parity(`Math.fround`). 클라우드(#54) `grade`/`maxEquipmentGrade` 0~6 → 0~7 검증 확장.
6. **HUD + 로컬라이즈**: 레어도 색 7개(Unique/Transcendent 신규 색, Theme 토큰) + 표시명 7개 ko 한글/en(일반~신화).
7. **소비 아이템**: 현재 소비 시스템 부재 — 레어도는 장비/룬만. 향후 소비 아이템 레어도 미부여(문서 명시).
8. **테스트**: 클라 Automation(마이그레이션 6→7 변환 전 매핑·기존 세이브 라운드트립·신규 등급 드롭/스탯/강화·도감 63셀·Uncommon 제거 회귀) + 서버 vitest(전 공식 7단계 parity + 마이그레이션). UE Build/Automation + 서버 build/test/lint **GREEN**, server-ci 포함 CI 그린([[feedback-ci-before-merge]]).

## 2. 범위 (In Scope)
### 2.1 enum + 마이그레이션 (character)
`EItemRarity` 7단계 + `FRarityMigration` + `ApplyFromSave` 변환 + SaveVersion 7.
### 2.2 전 공식 7단계 (character + backend 미러)
Uncommon 제거 + Unique/Transcendent 보간, 클라/서버 동시(parity).
### 2.3 룬 도감 63셀 확장 (character)
TotalCells/카테고리/행 보너스 7레어도 + 도감 마이그레이션.
### 2.4 클라우드 grade 확장 (backend)
0~6 → 0~7.
### 2.5 UI (designer)
레어도 색 7 + 표시명 7 ko/en.
### 2.6 밸런스 (balance)
7단계 드롭 곡선/스탯 배수/강화 비용/도감 행 보너스 보간 시뮬 + 파워크리프 가드.
### 2.7 테스트 — DoD 8.

## 3. 범위 외 (후속)
- 소비 아이템 레어도 · 레어도별 신규 콘텐츠(유니크/초월 전용 효과) · 레어도 표시명 prestige 초월과의 UI 맥락 추가 구분.

## 4. 작업 분배 — Codex 호출
| 파트 | 작업 | 비중 |
| --- | --- | --- |
| 캐릭터·전투 (메인, 대규모) | EItemRarity 7단계 + FRarityMigration + 전 공식 클라 보간(Uncommon 제거) + 도감 63셀 + 저장 v7 + 서버 미러 전 .ts + Automation | ✅ 메인 (`character`) |
| 백엔드 | 클라우드 grade 0~7 + 서버 공식 parity 보강(character 흡수 시 검증) | ✅ 보조 (`backend`) |
| 디자이너 | 레어도 색 7(Unique/Transcendent) + 표시명 7 ko/en + HUD 정합 | ✅ 보조 (`designer`) |
| 밸런스 | 7단계 드롭/스탯/강화/도감 행 보간 시뮬 + 파워크리프 가드 | ✅ 보조 (`balance`) |
| QA | 마이그레이션 6→7 매핑·기존 세이브 라운드트립·도감 63셀·Uncommon 제거 회귀 | ✅ 보조 (`qa`) |

## 5. 워크플로우 v3
[1] 기획+PR → [2] Codex character 메인(+backend/designer/balance/qa) → [3] Claude TM(parity 집중) → [4] Codex TM+fix → [5] 검증 → [N] **CI 그린 확정** + PM 종합 소견 + 머지.

## 6. 리스크
| 리스크 | 완화 |
| --- | --- |
| 전 공식 7단계 parity 누락(server-ci red) | grep `EItemRarity`/`ItemRarity` 전수 + 공식별 parity 테스트 + Math.fround. 누락 등급 케이스 컴파일 경고 활용 |
| 마이그레이션 회귀(기존 세이브 깨짐) | FRarityMigration 단위 테스트(6→7 전 매핑) + ApplyFromSave 라운드트립 + SaveVersion 게이트 |
| 도감 54→63 확장 마이그레이션 | 기존 셀 키 보존 + Uncommon 셀 키 Rare 병합 + 신규 행 미unlock + Automation |
| Uncommon 잔존 참조 | grep Uncommon 전수 제거(컴파일 에러로 검출) |
| 신규 등급 보간 밸런스 | balance 시뮬(드롭률 합=1.0 유지, 스탯 단조 증가, 페이싱 영향) |
| 클라우드 grade 범위 | 서버 validateSavePayload grade 0~7 + 기존 0~6 세이브 허용 |
| prestige 초월 표시명 겹침 | 레어도 "초월"은 아이템 패널 맥락, prestige 초월은 별도 패널(현 구조 유지) |

## 7. 후속
- 유니크/초월 전용 콘텐츠(세트/효과), 소비 아이템 레어도, 레어도 도감 심화.
