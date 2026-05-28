# PR #69 기획서 — 펫 시스템 대확장 (콘텐츠)

> **PM 자율 진행**(사용자 "PM 자율로 계속"). 펫은 #22(2종 dog/bird)·#42(레벨업)만 존재 → **종류 대폭 확장 + 보너스 타입 다양화**로 동반 시스템 심화([[project-content-richness]]). 직전 아이템/던전 메타와 다른 축. client + server 멀티시스템(+designer/balance). **PR #67 교훈: 스탯류 보너스는 % 곱(flat 아님)**.

## 1. 목표 / DoD
펫 종류가 2→10종으로 늘고 보너스 타입(골드/드롭/경험치/스탯류)이 다양해져, 펫 선택이 빌드 의미를 갖는다. 기존 펫(#22/#42 레벨업)·CP·재화·저장과 정합.

### DoD 검증
1. **데이터 모델**: `EPetBonusType` 확장 — None/Gold/Drop + **Exp/PhysAtk/MagicAtk/Hp/Def/AllStat**. 펫 카탈로그 2→**10종**(dog/bird + cat/wolf/owl/bear/turtle/fox/dragon/rabbit, 각 보너스 타입/값). `FPetDefinition`(기존 PetId/Name/BonusType/BonusPercent 유지).
2. **펫 해금**: 신규 펫은 골드 해금(`TryUnlockPet` 골드 게이트) 또는 기본 보유 정책(기존 InitializeDefaultPets 패턴 따름 — character 결정, 단 회귀안전). 기존 dog/bird 보유 유지.
3. **보너스 적용**:
   - Gold/Drop: 기존 `ApplyGoldBonus`/`ApplyDropBonusChance` 유지
   - **Exp**: 경험치 지급 훅(IdleMonster reward, 룬 #61 패턴) `×(1+petExp%)`
   - **스탯류(PhysAtk/MagicAtk/Hp/Def/AllStat)**: `RefreshDerivedStats`에서 **% 곱**(룬 코어 #61/유니크 trait #67 합산 곱 경로 — flat 아님, PR #67 교훈). 펫 레벨(#42) 배수 적용 유지.
4. **펫 레벨 연동**: 기존 FeedPet 레벨업(#42) 보너스 배수가 모든 신규 보너스 타입에 적용(GetEffectiveBonusPercent).
5. **저장**: OwnedPetIds(신규 펫 ID)/PetLevels 기존 직렬화. 기존 세이브 신규 펫 미보유(회귀안전). SaveVersion 필요 시 bump(펫 ID 문자열이라 마이그레이션 최소).
6. **서버 미러**: `server/src/core/formulas/petBonus.ts`(또는 기존 pet 모듈) 펫 카탈로그/보너스 공식 parity + test.
7. **HUD**: 펫 패널(10종 + 보너스/레벨/해금/장착) + 로컬라이즈 ko/en(펫명/보너스 타입).
8. **테스트**: 클라 Automation(펫 카탈로그 10·보너스 타입별 적용·스탯류 % 곱·해금/장착·레벨 배수·저장 라운드트립) + 서버 vitest(펫 보너스 parity). UE Build/Automation + 서버 build/lint/test **GREEN**, server-ci 그린.

## 2. 범위 (In Scope)
### 2.1 펫 데이터/카탈로그 (character + backend 미러)
EPetBonusType 확장 + 10종 카탈로그 + 서버 미러.
### 2.2 보너스 적용 (character)
Exp 훅 + 스탯류 % 곱(RefreshDerivedStats) + Gold/Drop 기존.
### 2.3 해금/장착 (character)
신규 펫 해금(골드 또는 기본) + 장착(기존).
### 2.4 저장 (character)
OwnedPetIds/PetLevels + 회귀안전.
### 2.5 서버 미러 (backend)
펫 보너스/카탈로그 parity test.
### 2.6 UI (designer)
펫 패널 10종 HUD + 로컬라이즈 ko/en.
### 2.7 밸런스 (balance)
펫 보너스 값/스탯류 % + 파워크리프 가드.
### 2.8 테스트 — DoD 8.

## 3. 범위 외 (후속)
- 펫 진화/합성 · 펫 등급(레어도) · 펫 스킬/액티브 · 펫 도감 · 다중 펫 장착.

## 4. 작업 분배 — Codex 호출
| 파트 | 작업 | 비중 |
| --- | --- | --- |
| 캐릭터·전투 (메인) | EPetBonusType 확장 + 10종 카탈로그 + 보너스 적용(Exp 훅/스탯류 % 곱/Gold·Drop) + 해금/장착 + 저장 + 서버 petBonus.ts + Automation | ✅ 메인 (`character`) |
| 디자이너 | 펫 패널 10종 HUD(보너스/레벨/해금/장착) + 로컬라이즈 ko/en | ✅ 보조 (`designer`) |
| 밸런스 | 펫 보너스 값/스탯류 % + 파워크리프 가드 | ✅ 보조 (`balance`) |
| (backend/qa) | character 흡수, [3] Claude TM parity·커버리지 | — |

## 5. 워크플로우 v3
[1] 기획+PR → [2] Codex character 메인(+designer/balance) → [3] Claude TM → [4] fix(필요시) → [5] 검증(**UE Automation 직접**, #68 교훈) → [N] **CI 그린 확정** + PM 종합 소견 + 머지. PM 자율.

## 6. 리스크
| 리스크 | 완화 |
| --- | --- |
| 스탯류 펫 보너스 flat 가산(#67 동류 단위 버그) | **% 곱**(RefreshDerivedStats 합산 곱) + 효과 % Automation 검증 |
| 펫 보너스 파워크리프 | balance 시뮬, 보너스 보수적, 레벨 배수(#42) 정합 |
| 클라/서버 펫 보너스 불일치 | petBonus.ts parity(Math.fround) + 카탈로그 동기 |
| 신규 펫 저장 회귀(기존 세이브) | 신규 펫 미보유 기본값 + 라운드트립 Automation |
| Exp 보너스 훅 누락 | IdleMonster reward 경로(룬 #61 패턴) 적용 + 테스트 |
| 펫 보너스 공식 변경 시 테스트/UE 누락(#68 교훈) | balance 변경 시 클라 테스트 동시 갱신 + [5] UE Automation 직접 |
| 클라우드(#54) 정합 | UStructToJson 자동(펫 필드) |

## 7. 후속
- 펫 진화/합성, 펫 등급, 펫 스킬, 펫 도감, 다중 장착.
