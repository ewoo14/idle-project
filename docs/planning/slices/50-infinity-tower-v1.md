# PR #50 기획서 — 무한의 탑 Infinity Tower (무한 콘텐츠)

> 무한 성장([[project-infinite-growth]])의 **도전 콘텐츠 트랙**. 전투력(CP, #49)이 끝없이 오르지만 그것을 시험할 별도 목표가 없다. **무한의 탑** — 층마다 요구 전투력이 오르는 끝없는 층 — 을 추가해, 플레이어 CP 가 닿는 만큼 자동으로 등반(방치형)하고 층 보상을 받는다. 챕터 스테이지 그라인드와 별개의 엔드 콘텐츠로 CP 성장에 명확한 도전 목표를 준다. client+server+UI+balance+qa 5-team.

## 1. 목표 / DoD
플레이어의 전투력으로 탑 층을 자동 등반하고(CP ≥ 층 요구 전투력이면 클리어→다음 층), 도달 최고 층과 층 보상을 얻는다.

### DoD 검증
1. 층 요구 전투력: `GetFloorRequiredPower(floor)` 무한 증가(예 base × growth^floor 또는 곡선). `CanClearFloor(combatPower, floor)` = CP ≥ 요구.
2. 자동 등반: `TryClimbTower(combatPower)` 가 CP 로 클리어 가능한 연속 층을 모두 클리어(HighestFloor 갱신)하고 신규 클리어 층 보상 합산 반환. CP 부족 시 정체.
3. 층 보상: 신규 클리어 층당 보상(예 골드, 층 비례). 결정적.
4. HUD 탑 패널(현재/최고 층·다음 층 요구 전투력·내 전투력·등반 상태).
5. 서버 TowerFormula 미러 + parity. 서버 Vitest + UE 빌드/Automation GREEN. 기존 시스템 회귀 없음(탑은 별도 트랙·읽기/보상).

## 2. 범위 (In Scope)
### 2.1 탑 공식 (메인, C++)
- `TowerFormula.h/.cpp`(GameCore/, 순수 static): `GetFloorRequiredPower(int32 Floor)` → int64(무한 증가 곡선, 예 round(100 × pow(1.15, Floor)) 또는 base+곡선). `CanClearFloor(int64 CombatPower, int32 Floor)` = CombatPower ≥ GetFloorRequiredPower(Floor). `GetFloorReward(int32 Floor)` → int64(층 비례 보상).
### 2.2 탑 상태/행위 (메인, C++)
- `UTowerService`(GameCore/, 서비스 패턴): `int32 HighestFloor`(0=미클리어). `int64 TryClimbTower(int64 CombatPower)`: floor = HighestFloor+1 부터 CanClearFloor 인 동안 반복 클리어(상한 가드 — 1회 호출당 최대 N층, 무한 루프 방지), HighestFloor 갱신, 신규 층 보상 합산 반환. `GetHighestFloor()/GetNextFloorRequiredPower()`. OnTowerClimbed 델리게이트.
- `UIdleGameInstance`: TowerService 보유(서비스 패턴) + GetTowerService + 등반 보상 골드 적용(AddGold) 경로. 주기/수동 등반 트리거(HUD 버튼 또는 전투 틱) — V1 HUD 버튼 "등반"으로 TryClimbTower(캐릭터 GetCombatPower) 호출 후 보상 골드 지급.
### 2.3 UI (디자이너)
- HUD 탑 패널: 최고 층, 다음 층 요구 전투력, 내 전투력(GetCombatPower), 등반 버튼(HitBox). 등반 결과(클리어 층수/보상) 피드백. 로컬라이즈 ko/en.
### 2.4 서버 (백엔드)
- `server/src/core/formulas/tower.ts`: getFloorRequiredPower/canClearFloor/getFloorReward 클라 미러(Math.fround float) + parity 테스트.
### 2.5 데이터/밸런스
- 층 요구 전투력 곡선·층 보상 + CP 성장(#49) 대비 등반 곡선 + 골드 보상 경제 영향(강화 싱크 #44 와 균형) + 문서.
### 2.6 테스트
- 서버 Vitest(요구/클리어/보상 곡선·미러·parity) + 클라 Automation(GetFloorRequiredPower 증가, CanClearFloor 경계, TryClimbTower 연속 클리어+상한+보상 합산, CP 부족 정체).

## 3. 범위 외
- 실시간 탑 전투 인스턴스(V1 CP 자동 판정), 탑 전용 보스 메커닉/룩, 탑 랭킹, 탑 전용 재화/상점(후속).
- 서버 권위 탑(클라 권위 V1, 서버 공식 미러만).

## 4. 작업 분배 — Codex 호출
| 파트 | 작업 | 비중 |
| --- | --- | --- |
| 캐릭터·전투 (메인) | TowerFormula + UTowerService + GameInstance 등반/보상 + Automation | ✅ 메인 (`character`) |
| 백엔드 | tower.ts 미러 + parity | ✅ 보조 (`backend`) |
| 디자이너 | 탑 패널 HUD + 로컬라이즈 | ✅ 보조 (`designer`) |
| 밸런스 | 요구/보상 곡선 + CP 대비 등반 + 경제 영향 + 문서 | ✅ 보조 (`balance`) |
| QA | 등반/요구/보상/정체 시나리오 | ✅ 보조 (`qa`) |

## 5. 워크플로우 v3
[1] 기획+PR → [2] Codex character 메인(+backend/designer/balance/qa) → [3] Claude TM → [4] Codex TM+fix → [5] 검증 → [N] **CI 그린 확정** + 머지. 사용자 PIE 차후([[feedback-autonomous-slices]]). 머지 전 CI 별도 확인([[feedback-ci-before-merge]], [[reference-ci-retrigger]]). **Codex 커밋/푸시 누락 PM 보완**([[reference-codex-invoke]]).

## 6. 리스크
| 리스크 | 완화 |
| --- | --- |
| TryClimbTower 무한 루프(CP 폭주) | 1회 호출 최대 클리어 층 상한(예 100) + 요구 전투력 단조 증가 |
| 층 보상 골드 인플레 | 보상 곡선 보수적 + 강화 싱크(#44) 대비 balance 점검 |
| 서버↔클라 곡선 parity | TowerFormula Math.fround float + pow 경계 테스트 |
| 큰 수 오버플로(고층 요구/보상) | int64 + pow 상한/클램프 |
| 기존 시스템 회귀 | 탑은 별도 TowerService·읽기/보상, 전투/스탯 무변경 |

## 7. 후속
- 실시간 탑 전투, 탑 전용 보스/보상/재화, 탑 랭킹, 서버 권위 탑.
