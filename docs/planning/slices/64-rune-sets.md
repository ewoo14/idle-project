# PR #64 기획서 — 룬 세트 효과 (룬 시스템 확장 3)

> **사용자 선택(룬 확장 계속)**: 룬 트랙 4번째(#61 장착/#62 도감/#63 직업룬). 범용 6슬롯에 같은 테마 룬을 모으는 **조합 빌드 깊이**. 장비 세트(#43)와 독립된 룬 전용 세트 + 2/4/6 티어 보너스. 콘텐츠 볼륨([[project-content-richness]]) + 빌드 다양성. 클라/서버 미러(parity). client + server 멀티시스템(+designer/balance).

## 1. 목표 / DoD
범용 6슬롯에 같은 룬 세트를 2/4/6개 모으면 tiered 보너스를 얻는다. 기존 룬(#61~#63)·장비 세트(#43)·스탯 합성과 정합.

### DoD 검증
1. **데이터 모델**: `ERuneSet`(신규 4종: Offense/Bastion/Vitality/Fortune). `FRuneInstance`에 `ERuneSet RuneSet = None(0)` 추가(기존 룬 None 직렬화 호환). 집계 대상은 **범용 6슬롯(0~5)** 코어/유틸 룬만(직업 전용 슬롯6#63 제외, ClassMastery 룬은 세트 None).
2. **세트 보너스(2/4/6 티어, flat 유한, 단계별=도달 티어 값 적용·누적 아님)**: `FRuneSetFormula::ComputeSetBonus(장착 룬 배열)` → `FRuneCoreMultipliers`(코어 가산) + `FRuneUtilValues`(유틸 가산). 같은 세트 장착 개수의 **최고 도달 티어 값**:
   | 세트 | 효과 | 2개 | 4개 | 6개 |
   | --- | --- | --- | --- | --- |
   | Offense | 물공·마공(코어) | +0.05 | +0.12 | +0.25 |
   | Bastion | 물방·마방(코어) | +0.05 | +0.12 | +0.25 |
   | Vitality | 체력(코어)+오프라인효율(유틸) | +0.05 | +0.12 | +0.25 |
   | Fortune | 골드획득·경험치·치명피해(유틸) | +0.05 | +0.12 | +0.25 |
   - 예: Offense 6개 = +0.25(누적 0.42 아님). 초기 수치 balance 시뮬 튜닝. 무한 아님(룬 강화 #61이 무한 담당).
3. **드롭/부여**: 룬 드롭/상점 롤 시 세트 부여(레어도 비례 None~세트, 장비 #43 `RollItemSet` 패턴). ClassMastery 룬은 세트 None.
4. **스탯 합성 통합**: `GetEquippedCoreMultipliers`에 세트 코어 보너스 `+=` 합산(기존 합 경로). `GetEquippedUtilValues`에 세트 유틸 가산(**캡 외 별도 가산** 정책 — 세트 보너스는 유틸 캡에 막히지 않음, balance 확정). 세트 None/미충족 → 0(회귀안전).
5. **저장(`SaveVersion` 5→6)**: `FRuneInstance.RuneSet` 직렬화 + v5→v6 마이그레이션 회귀안전(세트 None=보너스 0). 클라우드(#54) 자동 정합.
6. **서버 미러**: `server/src/core/formulas/runeSet.ts` + `runeSet.test.ts`(세트 임계 2/4/6·보너스 parity, `Math.fround`).
7. **HUD**: 룬 패널에 세트 현황(세트별 장착 개수 N/6 + 활성 티어 표시) — 장비 세트 HUD(#43) 패턴 재사용 + 로컬라이즈 ko/en.
8. **테스트**: 클라 Automation(세트 부여·2/4/6 티어 보너스·범용 슬롯 한정(직업 슬롯 제외)·세트 None 회귀안전·저장 v5→v6 마이그레이션·코어/유틸 가산) + 서버 vitest(`runeSet.ts` 임계/보너스 parity). UE Build/Automation + 서버 build/test/lint **GREEN**, server-ci 포함 CI 그린.

## 2. 범위 (In Scope)
### 2.1 룬 세트 데이터/공식 (character + backend 미러)
`ERuneSet` + `FRuneInstance.RuneSet` + `FRuneSetFormula`(임계/보너스/RollRuneSet) + 서버 `runeSet.ts`.
### 2.2 세트 집계 + 합성 (character)
`GetEquippedCoreMultipliers`/`GetEquippedUtilValues`에 범용 6슬롯 세트 집계 후 보너스 합산.
### 2.3 드롭 부여 (character)
`RollShopRune`/드롭에 세트 롤(ClassMastery 제외).
### 2.4 저장 v6 (character)
`RuneSet` 직렬화 + v5→v6 마이그레이션.
### 2.5 서버 미러 (backend)
`runeSet.ts` + parity test.
### 2.6 UI (designer)
세트 현황 HUD + 로컬라이즈 ko/en.
### 2.7 밸런스 (balance)
세트 2/4/6 보너스 수치 시뮬 + 파워크리프 가드 + 페이싱 영향.
### 2.8 테스트 — DoD 8.

## 3. 범위 외 (후속)
- 직업 전용 룬 세트 · 세트 전용 슬롯 · 세트 변환/리롤 아이템 · 룬 세트 도감 연계 · 강화 단계 도감.

## 4. 작업 분배 — Codex 호출
| 파트 | 작업 | 비중 |
| --- | --- | --- |
| 캐릭터·전투 (메인) | ERuneSet/FRuneInstance.RuneSet/FRuneSetFormula + 세트 집계·합성(범용 6슬롯) + 드롭 부여 + 저장 v6 + 서버 runeSet.ts + Automation | ✅ 메인 (`character`) |
| 디자이너 | 세트 현황 HUD(개수/티어) + 로컬라이즈 ko/en | ✅ 보조 (`designer`) |
| 밸런스 | 세트 2/4/6 수치 시뮬 + 파워크리프 가드 + 페이싱(median) 영향 | ✅ 보조 (`balance`) |
| (backend/qa) | character 흡수 가능, [3] Claude TM parity·커버리지 점검 | — |

## 5. 워크플로우 v3
[1] 기획+PR → [2] Codex character 메인(+designer/balance) → [3] Claude TM → [4] fix(필요시) → [5] 검증 → [N] **CI 그린 확정** + PM 종합 소견 + 머지. 사용자 PIE 차후([[feedback-autonomous-slices]]).

## 6. 리스크
| 리스크 | 완화 |
| --- | --- |
| 세트 집계 범위 오류(직업 슬롯 포함) | 범용 슬롯 0~5만 집계, ClassMastery/직업 슬롯 제외 + Automation |
| 장비 세트(#43)와 혼동 | `ERuneSet` 별도 enum + 룬 전용 공식, 장비 `EItemSet` 무관 |
| 세트 파워크리프(강화 무한 + 세트 가산) | flat 유한 2/4/6, balance 시뮬 페이싱 확인 |
| 유틸 세트 보너스 캡 상호작용 | 세트 보너스는 캡 외 별도 가산(정책 명시) + 경계 Automation |
| 클라/서버 임계·보너스 불일치 | runeSet.ts parity(Math.fround) + 2/4/6 앵커 |
| v5→v6 마이그레이션 회귀 | 세트 None/보너스 0 기본값 + 라운드트립 Automation |
| 드롭 결정성(세트 롤) | RandomStream 시드 순서 보존(장비 #43 RollItemSet 후 추가 시 순서 주의) |
| 클라우드(#54) 정합 | UStructToJson 자동 포함(RuneSet 필드) |

## 7. 후속 (룬 확장 계속)
- 룬 리롤/전송, 강화 단계 도감, 직업 전용 룬 세트, 세트 변환 아이템, 룬 세트 도감 연계.
