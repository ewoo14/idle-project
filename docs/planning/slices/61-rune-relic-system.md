# PR #61 기획서 — 룬/유물 장착 시스템 (신규 메타 성장)

> **사용자 선택(신규 메타 시스템): 룬/유물 장착.** #58 "범위 외(후속)"에 명시된 **룬/소켓 시스템**의 실현이자, 장비(8슬롯)·affix(#40)·세트(#43)와 **차별화된 신규 스탯 소스 + 무한 성장 벡터**([[project-infinite-growth]]). 장비와 별개의 전용 룬 슬롯 6칸에 룬을 장착하고, 룬 자체를 전용 재화로 **무한 강화**한다. 코어 룬은 스탯별 % 무한 곱 배수(`RefreshDerivedStats` 합성에 안전 합류), 유틸 룬은 flat 가산 + 명시적 캡으로 밸런스를 통제. 드롭 + 분해 → 룬 에센스(전용 재화) 경제로 골드 싱크 + 신규 재화 루프 동시 확보. 클라/서버 미러(parity). client + server 멀티시스템(+designer/balance/qa).

## 1. 목표 / DoD
장비와 독립된 룬 슬롯 6칸에 룬을 장착·강화·분해하여, 무한 성장(코어 룬 % 곱)과 빌드 다양성(코어/유틸 혼합 + 슬롯 선택)을 얻는다. 기존 스탯 합성(Transcend×Tower×Achievement)·저장(#52~#54)·경제(강화 골드 싱크)와 정합.

### DoD 검증
1. **데이터 모델**: `ERuneType` 9종 — 코어 5(PhysAtk/MagicAtk/PhysDef/MagicDef/Hp) + 유틸 4(CritDamage/GoldFind/ExpBoost/OfflineEff). 범용 6슬롯(타입 제한 없음). 레어도는 기존 `EItemRarity`(Common~Mythic) 재사용. `FRuneInstance`(RuneId/RuneType/Rarity/EnhanceLevel 무한).
2. **공식(무한 성장 + 캡)**: `FRuneFormula` — 코어 배수 `base(rarity) + lvl×step(rarity)`(무한 증가), 유틸 값 `base + lvl×step` + **타입별 캡**(rate/획득률 폭주 방지), 강화 비용 에센스·골드 `base×(lvl+1)²`(기하급수 싱크), 분해 에센스 산출, 드롭 롤(보스 우대), 상점 뽑기 골드 비용. 초기 수치는 **balance가 시뮬로 튜닝**.
3. **서비스**: `URuneService`(기존 U…Service 패턴) — OwnedRunes/EquippedRuneSlots(6)/RuneEssence. `AddRune`/`TryEquipRune`/`UnequipRune`/`TryEnhanceRune`(에센스+골드 3중 가드 1회 차감)/`TryDisenchantRune`(장착중 거부)/`GetEquippedCoreMultipliers()`(스탯별 5)/`GetEquippedUtilValues()`(캡 적용). `CaptureState`/`RestoreState`(무효 필터 + 장착 인덱스 재매핑, #53 교훈).
4. **스탯 합성 통합**: `RefreshDerivedStats`에서 기존 단일 `StatMultiplier` 곱(`IdleCharacter.cpp:181-185`) 유지 + 룬 코어 배수 **스탯별 추가 곱**(`Derived.PhysAtk *= (1 + RuneMult.PhysAtk)` … 5스탯). 유틸: CritDamage→`Derived.CritDmg` 가산 / GoldFind·ExpBoost→보상(reward) 조회 / OfflineEff→오프라인 공식 조회. 룬 미장착 ×1.0·가산 0 회귀안전.
5. **저장(SaveVersion 2→3)**: `UIdleSaveGame`에 `Runes`/`EquippedRuneSlots`/`RuneEssence` 추가. `CaptureToSave`/`ApplyFromSave` 연동. **v<3 마이그레이션 회귀안전**(빈 룬/0 에센스). 클라우드(#54) `clientSave` 중첩 자동 포함(서버 스키마 무변경).
6. **서버 미러**: `server/src/core/formulas/rune.ts` + `rune.test.ts`(앵커값 + `Math.fround` parity + 캡 경계). 클라 `FRuneFormula`와 exact parity.
7. **HUD**: `FIdleHUDRuneViewModel`(6슬롯 타입/레어도/강화레벨/현재값, 보유 목록, 에센스, 강화 미리보기) + 룬 패널(장착·해제·강화·분해·상점뽑기, 기존 HUD 입력 패턴) + 로컬라이즈 `Rune.csv`(ko/en 룬 타입명/UI 라벨).
8. **테스트**: 클라 Automation(장착/강화 단조증가/분해/스탯합성/저장 라운드트립/v2→v3 마이그레이션/유틸 캡 경계/장착중 분해 거부/결정적 드롭 RNG) + 서버 vitest(rune.ts 앵커 + parity + 캡) + CsvIntegrity. UE 빌드/Automation + 서버 build/test/lint GREEN. **server-ci 포함 CI 그린 확정**([[feedback-ci-before-merge]]).

## 2. 범위 (In Scope)
### 2.1 룬 데이터/공식 (character + backend 미러)
- `ERuneType`(9) + `FRuneInstance` + `FRuneFormula`(코어 배수/유틸 값+캡/강화 비용/분해/드롭/상점). 서버 `rune.ts` 동기.
### 2.2 룬 서비스 (character)
- `URuneService` 생성 + GameInstance 등록(`EnsureRuneService`/getter/Shutdown) + 장착·강화·분해 API(골드 게이트는 GameInstance 경유, 3중 가드 1회 차감).
### 2.3 스탯 합성 통합 (character)
- `RefreshDerivedStats` 룬 코어 스탯별 곱 + 유틸 가산/훅(reward·offline). 회귀안전.
### 2.4 저장 (character)
- SaveVersion 2→3 + 신규 필드 + Capture/Apply + 마이그레이션 회귀안전 + 클라우드 정합.
### 2.5 서버 미러 (backend)
- `rune.ts` 전 공식 미러 + `rune.test.ts` parity/캡. 클라우드 payload 정합 확인.
### 2.6 UI (designer)
- `FIdleHUDRuneViewModel` + 룬 패널 + 로컬라이즈 ko/en.
### 2.7 밸런스 (balance)
- 코어 base/step, 유틸 base/step/캡, 강화 비용(에센스·골드), 드롭률, 상점 비용 시뮬 튜닝. 골드·에센스 싱크 검증 + 파워크리프 가드(기존 페이싱 median 영향 점검).
### 2.8 테스트 — 위 DoD 8.

## 3. 범위 외 (후속)
- 룬 세트 효과(장비 세트 #43과 중복 — 제외), 룬 랜덤 affix(#40과 중복 — 룬은 타입 고정 스탯), 룬 보드/그리드 시너지(오버킬), 룬 도감 UI, 직업 전용 룬, 룬 잠금/일괄 분해 편의, `RuneDB.csv`(공식 상수로 충분 — YAGNI).

## 4. 작업 분배 — Codex 호출
| 파트 | 작업 | 비중 |
| --- | --- | --- |
| 캐릭터·전투 (메인) | ERuneType/FRuneInstance/FRuneFormula + URuneService + GameInstance 등록 + RefreshDerivedStats 통합 + 저장(v3) + Automation | ✅ 메인 (`character`) |
| 백엔드 | 서버 `rune.ts` 미러 + parity vitest + 클라우드 payload 정합 | ✅ 보조 (`backend`) |
| 디자이너 | 룬 패널 ViewModel + 로컬라이즈 ko/en(룬 타입명/UI 라벨) | ✅ 보조 (`designer`) |
| 밸런스 | 코어/유틸/강화/드롭/상점 수치 시뮬 튜닝 + 파워크리프 가드 | ✅ 보조 (`balance`) |
| QA | 장착·강화·분해·스탯합성·저장·마이그레이션·캡·드롭 시나리오 | ✅ 보조 (`qa`) |

## 5. 워크플로우 v3
[1] 기획+PR → [2] Codex character 메인(+backend/designer/balance/qa) → [3] Claude TM → [4] Codex TM+fix → [5] 검증(UE Build+Automation / 서버 build/test/lint) → [N] **CI 그린 확정**(server-ci 포함, [[feedback-ci-before-merge]]) + PM 종합 소견 + 머지. 사용자 PIE 차후([[feedback-autonomous-slices]]). [[project-infinite-growth]].

## 6. 리스크
| 리스크 | 완화 |
| --- | --- |
| 코어 룬 % 무한 곱 파워크리프 | balance 시뮬(페이싱 median 영향), 코어/유틸 분리, base/step 보수적 시작 |
| 유틸 룬 rate/획득률 폭주(크리·골드·경험치) | flat 가산 + **타입별 명시 캡**, 캡 경계 Automation/parity 테스트 |
| 클라/서버 공식 불일치 | `rune.ts` parity(`Math.fround`) + 앵커값 + DefinitionParity |
| 저장 v2→v3 마이그레이션 회귀 | else 분기 빈 룬/0 에센스 기본값 + 라운드트립/마이그레이션 Automation |
| 장착 인덱스 무결성(분해/필터 후) | RestoreState 장착 인덱스 재매핑(#53 교훈) + 장착중 분해 거부 |
| 클라우드(#54) 정합 | `clientSave` 중첩 룬 직렬화 round-trip, 서버 스키마 무변경 확인 |
| 골드/에센스 싱크 부족 또는 과잉 | 강화 비용 기하급수(`(lvl+1)²`) + 분해 환급 균형 시뮬 |
| 결정적 드롭 RNG 깨짐 | RandomStream 시드 순서 보존, 결정성 Automation |

## 7. 후속 (메타 성장 계속)
- 룬 도감/수집 보너스, 직업 전용 룬, 룬 세트(별도 설계 시), 룬 리롤/전송, 자동 장착 최적화. (신규 메타 시스템 첫 슬라이스 — 룬 장착 기반 위 확장.)
