# PR #51 기획서 — 탑 마일스톤 영구 보너스 (무한 성장, 경제 중립)

> 무한의 탑(#50)은 등반 시 골드만 준다. 탑 등반에 **지속(영구) 가치**를 주기 위해, 최고 도달 층 N층 돌파마다 **영구 글로벌 스탯 배수**(파워, 골드 아님 → 경제 중립)를 부여한다. 탑 등반 → 영구 파워 성장 → 더 높은 층 → … 무한 루프. 골드 인플레(문서화된 한계)를 피하려 파워 기반 보상을 택한다. 초월(#47) 배수와 별개로 곱해진다. client+server+UI+balance+qa 5-team.

## 1. 목표 / DoD
탑 최고 도달 층이 높을수록 영구 글로벌 스탯 배수가 커져 전 전투 스탯(및 전투력 CP)이 강해진다.

### DoD 검증
1. `GetTowerMilestoneMultiplier(HighestFloor)` = 1 + floor(HighestFloor/10) × 0.02 (10층마다 +2% 글로벌, 무한). 0~9층 = ×1.0.
2. RefreshDerivedStats 가 최종 Derived 의 Hp/PhysAtk/MagicAtk/PhysDef/MagicDef 에 탑 마일스톤 배수 곱(rate 류 제외, 클램프 보존). 초월(#47) 배수와 별개(둘 다 곱).
3. 0층(미등반) = ×1.0 → 기존 스탯 불변(회귀 안전). 탑 등반(#50)으로 HighestFloor↑ → CP↑.
4. HUD(탑 패널 또는 정보 패널)에 현재 마일스톤 배수/다음 마일스톤 표시. 서버 미러 + parity. 서버 Vitest + UE 빌드/Automation GREEN.

## 2. 범위 (In Scope)
### 2.1 마일스톤 공식 (메인, C++)
- `TowerMilestoneFormula.h/.cpp`(GameCore/, 순수 static): `GetTowerMilestoneMultiplier(int32 HighestFloor)` = 1.0 + (max(0,HighestFloor)/10) × 0.02 (정수 나눗셈 floor, 무한 선형). `GetMilestoneStep`(10), `GetMilestoneBonusPerStep`(0.02) 상수.
### 2.2 적용 (메인, C++)
- `UTowerService`(#50): GetHighestFloor() 활용. `float GetMilestoneMultiplier() const` = GetTowerMilestoneMultiplier(HighestFloor).
- `UIdleGameInstance`: `float GetTowerMilestoneMultiplier() const`(TowerService 경유).
- `AIdleCharacter::RefreshDerivedStats`: 초월 배수 적용 후(또는 함께) 탑 마일스톤 배수를 Hp/PhysAtk/MagicAtk/PhysDef/MagicDef 에 곱. HighestFloor 0 → ×1.0.
### 2.3 UI (디자이너)
- HUD 탑 패널(#50) 또는 정보 패널(#41)에 "탑 보너스 ×N.NN"(현재) + 다음 마일스톤(층) 표시. 로컬라이즈 ko/en.
### 2.4 서버 (백엔드)
- `server/src/core/formulas/towerMilestone.ts`: getTowerMilestoneMultiplier(highestFloor)(Math.fround float parity) 미러 + parity 테스트.
### 2.5 데이터/밸런스
- 마일스톤 스텝(10층)·보너스(+2%) + 초월(#47)과 복합 곱 + CP(#49) 영향 + 경제 중립(파워만) 문서.
### 2.6 테스트
- 서버 Vitest(배수/미러/parity) + 클라 Automation(GetTowerMilestoneMultiplier 경계 0/9=1.0·10=1.02·100=1.20, RefreshDerivedStats 배수 반영, 0층 회귀, 초월 배수와 복합 곱).

## 3. 범위 외
- 탑 마일스톤 특별 보상(아이템/재화)(후속), 마일스톤 연출, 탑 전용 보너스 종류 확장.
- 서버 권위 탑 마일스톤(클라 권위 V1).

## 4. 작업 분배 — Codex 호출
| 파트 | 작업 | 비중 |
| --- | --- | --- |
| 캐릭터·전투 (메인) | TowerMilestoneFormula + TowerService/GameInstance 노출 + RefreshDerivedStats 곱 + Automation | ✅ 메인 (`character`) |
| 백엔드 | towerMilestone.ts 미러 + parity | ✅ 보조 (`backend`) |
| 디자이너 | 탑/정보 패널 마일스톤 배수 표시 + 로컬라이즈 | ✅ 보조 (`designer`) |
| 밸런스 | 스텝/보너스 + 초월 복합 + 경제 중립 + 문서 | ✅ 보조 (`balance`) |
| QA | 마일스톤 배수/회귀/복합 시나리오 | ✅ 보조 (`qa`) |

## 5. 워크플로우 v3
[1] 기획+PR → [2] Codex character 메인(+backend/designer/balance/qa) → [3] Claude TM → [4] Codex TM+fix → [5] 검증 → [N] **CI 그린 확정** + 머지. 사용자 PIE 차후([[feedback-autonomous-slices]]). 머지 전 CI 별도 확인([[feedback-ci-before-merge]], [[reference-ci-retrigger]]). **Codex 커밋/푸시 누락 PM 보완**([[reference-codex-invoke]]).

## 6. 리스크
| 리스크 | 완화 |
| --- | --- |
| 초월(#47)·탑 마일스톤 배수 중복 적용/위치 | RefreshDerivedStats 단일 지점에서 두 배수 명시적 곱(rate 제외), 0층/0초월 = ×1.0 회귀 테스트 |
| 배수 폭주 | +2%/10층 보수적 선형 + balance, CP(#49)로 체감 |
| 서버↔클라 parity | towerMilestone.ts Math.fround float + 경계 테스트 |
| 기존 스탯/전투 회귀 | HighestFloor 0 → ×1.0, 기존 불변 |

## 7. 후속
- 탑 마일스톤 특별 보상/재화, 마일스톤 연출, 보너스 종류 확장, 서버 권위.
