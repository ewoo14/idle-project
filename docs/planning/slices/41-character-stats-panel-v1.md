# PR #41 기획서 — 캐릭터 스탯 정보 패널 (심화)

> 빌드에 영향을 주는 스탯 소스가 4개(직업 성장·스탯 분배 #34·장비 affix #40·환생 포인트)로 늘었지만, 플레이어가 **자신의 최종 능력치(ATK/HP/크리/공속 등)를 볼 화면이 없다**. 캐릭터 정보 패널을 추가해 1차/2차 능력치를 한 화면에 노출, 분배·장비·affix·환생의 효과를 확인할 수 있게 한다. 게임 로직/경제 무변경(읽기/표시 전용).

## 1. 목표 / DoD
플레이어가 패널을 열어 현재 1차 능력치(STR/DEX/INT/WIS/CON/LUK, 분배 포함)와 2차 능력치(HP/ATK/마법ATK/DEF/마법DEF/공속/크리율/크리뎀/회피/명중)를 확인할 수 있다.

### DoD 검증
1. 캐릭터 스탯 패널(토글)에 1차 6종(분배 반영) + 2차 주요(HP/ATK/MATK/DEF/MDEF/AtkSpeed/Crit%/CritDmg/Dodge/Accuracy) 표시.
2. 표시 값이 실제 적용 스탯과 일치(직업 성장 + 분배 #34 + 장비/affix #40 + 환생 포인트 → DeriveStats 결과).
3. 분배/장착/affix/레벨업 변화 시 패널이 최신 값 반영.
4. UE 빌드/Automation(뷰모델) GREEN. 게임 로직/전투/경제 회귀 없음.

## 2. 범위 (In Scope)
### 2.1 스탯 접근자 (메인, C++)
- `AIdleCharacter`: RefreshDerivedStats 에서 계산한 `FPrimaryStats`(분배 포함)·`FDerivedStats` 를 캐싱하고 `GetCurrentPrimaryStats()`/`GetCurrentDerivedStats()`(BlueprintPure) 노출. (또는 동일 입력으로 재계산하는 순수 헬퍼.) 기존 RefreshDerivedStats 로직 재사용(중복 계산 경로 금지).
### 2.2 정보 패널 (디자이너, C++ HUD)
- HUD 캐릭터 정보 패널(토글 — 기존 메뉴 토글 또는 신규 HitBox/키): 1차 6종 + 2차 주요 라벨/값. ViewModel(예 FIdleHUDStatInfoViewModel BuildStatInfoViewModel(Primary, Derived)) 순수 함수 포맷(% 는 크리율/회피/명중, 배수는 크리뎀). 직업명/레벨/환생 횟수 헤더(선택). 로컬라이즈 ko/en(스탯 라벨).
### 2.3 테스트
- 뷰모델 포맷/값 매핑 Automation(1차/2차 표기, % vs 소수 vs 정수). 스탯 접근자 일관성(RefreshDerivedStats 결과와 동일).

## 3. 범위 외
- 스탯 툴팁/상세 설명, 스탯 비교(장착 전후), DPS 추정치(후속).
- 신규 스탯/공식(표시 전용, 공식 무변경).
- 서버 연동(클라 표시 전용).

## 4. 작업 분배 — Codex 호출
| 파트 | 작업 | 비중 |
| --- | --- | --- |
| 캐릭터·전투 (메인) | 스탯 캐싱/접근자(GetCurrentPrimary/DerivedStats) + Automation | ✅ 메인 (`character`) |
| 디자이너 | 정보 패널 ViewModel/렌더/토글 + 로컬라이즈 | ✅ 보조 (`designer`) |
| QA | 패널 표시/값 일치/갱신 시나리오 | ✅ 보조 (`qa`) |

## 5. 워크플로우 v3
[1] 기획+PR → [2] Codex character 메인(+designer/qa) → [3] Claude TM → [4] Codex TM+fix → [5] 검증 → [N] **CI 그린 확정** + 머지. 사용자 PIE 차후([[feedback-autonomous-slices]]). 머지 전 CI 별도 확인([[feedback-ci-before-merge]]). **Codex 커밋/푸시 누락 PM 보완**([[reference-codex-invoke]]).

## 6. 리스크
| 리스크 | 완화 |
| --- | --- |
| 표시 값과 실제 적용 스탯 불일치 | RefreshDerivedStats 와 동일 경로 재사용/캐싱 + 접근자 일관성 테스트 |
| 패널 토글이 기존 입력(메뉴/퀘스트 로그)과 충돌 | 별도 HitBox/키 또는 기존 메뉴 패널에 탭 추가, 기존 토글 회귀 없음 |
| DrawHUD 패널 누적/레이아웃 | 기존 패널 레이아웃 패턴 재사용, 토글 시만 그림 |
| 게임 로직 회귀 | 읽기/표시 전용, 공식/전투/경제 무변경 |

## 7. 후속
- 스탯 툴팁/설명, 장착 전후 비교, DPS 추정, 스탯별 기여 분해.
