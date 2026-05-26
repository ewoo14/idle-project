# PR #46 기획서 — 환생 보상 스케일링 (무한 prestige)

> 무한 성장([[project-infinite-growth]]) prestige 백본. 환생(#19)은 매번 **+5 영구 포인트 고정**이라 반복 가치가 정체된다. **환생 횟수·도달 레벨에 비례**해 보상(영구 포인트)이 커지게 해, 더 깊이 진행 후 환생할수록·여러 번 환생할수록 강해지는 무한 prestige 루프를 만든다. RebirthBonusPoints 는 DeriveStats(HP+10·PhysAtk+2/pt)로 영구 성장. client+server+UI+balance+qa 5-team.

## 1. 목표 / DoD
환생 시 받는 영구 포인트가 환생 횟수와 환생 시점 레벨에 비례해 커지고, 환생 화면에서 미리 확인할 수 있다.

### DoD 검증
1. 환생 보상 = `GetRebirthPointsReward(rebirthCount, level)` — 횟수↑·레벨(100 초과분)↑ → 보상↑. (1회차 Lv100 = 5, 기존 회귀.)
2. Rebirth() 가 RebirthBonusPoints += 보상(스케일). 기존 리셋(레벨/분배/골드 10%/스테이지) 유지.
3. CanRebirth 는 기존(보스+Lv100). **단 Lv100 초과 후 환생하면 보상↑**(더 진행할 동기).
4. HUD 환생 패널이 "이번 환생 보상 +N 포인트" 미리보기 표시.
5. 서버 RebirthFormula 미러 + parity. 서버 Vitest + UE 빌드/Automation GREEN. 1회차 회귀 없음.

## 2. 범위 (In Scope)
### 2.1 환생 공식 (메인, C++)
- `RebirthFormula.h/.cpp`(GameCore/, 순수 static): `GetRebirthPointsReward(int32 RebirthCount, int32 LevelAtRebirth)` → int32. 예 `5 + RebirthCount×2 + max(0, floor((LevelAtRebirth-100)/10))`. (RebirthCount=현재값(증가 전); 1회차 count 0·Lv100 = 5 회귀.) 무한 증가(상한 없음).
### 2.2 환생 적용 (메인, C++)
- `UIdleGameInstance::Rebirth()`: `const int32 Reward = FRebirthFormula::GetRebirthPointsReward(RebirthCount, CharacterLevel); ++RebirthCount; RebirthBonusPoints += Reward;` (기존 +5 대체). 나머지 리셋 동일.
- `PreviewRebirthReward()`(BlueprintPure) → GetRebirthPointsReward(RebirthCount, CharacterLevel)(현재 환생 시 받을 보상, HUD 용).
### 2.3 UI (디자이너)
- HUD 환생 패널: 기존 표시(상태/보스/레벨/횟수/현재 보너스 포인트)에 **이번 환생 보상 미리보기**(PreviewRebirthReward, "+N 포인트") 추가. 로컬라이즈 ko/en.
### 2.4 서버 (백엔드)
- `server/src/core/formulas/rebirth.ts`: getRebirthPointsReward(rebirthCount, levelAtRebirth) 클라 미러 + parity 테스트.
### 2.5 데이터/밸런스
- 보상 스케일 계수(횟수×2, 레벨/10) + 환생 도달 시간(balance-sim #23) 대비 prestige 성장 곡선 점검 + 문서.
### 2.6 테스트
- 서버 Vitest(보상 공식/미러/parity) + 클라 Automation(보상 스케일: 1회차 Lv100=5·다회차·고레벨 증가, Rebirth 적용, Preview 일치, 기존 리셋 유지).

## 3. 범위 외
- 환생 전용 상점/재화(환생 토큰), 초월(환생 위 단계), 환생 보너스 종류 확장(현재 영구 포인트만)(후속).
- CanRebirth 조건 변경(보스+Lv100 유지). 서버 권위 환생(클라 권위 V1).

## 4. 작업 분배 — Codex 호출
| 파트 | 작업 | 비중 |
| --- | --- | --- |
| 캐릭터·전투 (메인) | RebirthFormula + Rebirth() 스케일 적용 + Preview + Automation | ✅ 메인 (`character`) |
| 백엔드 | rebirth.ts 미러 + parity | ✅ 보조 (`backend`) |
| 디자이너 | 환생 패널 보상 미리보기 + 로컬라이즈 | ✅ 보조 (`designer`) |
| 밸런스 | 보상 스케일 계수 + prestige 곡선 + 문서 | ✅ 보조 (`balance`) |
| QA | 보상 스케일/1회차 회귀/Preview 시나리오 | ✅ 보조 (`qa`) |

## 5. 워크플로우 v3
[1] 기획+PR → [2] Codex character 메인(+backend/designer/balance/qa) → [3] Claude TM → [4] Codex TM+fix → [5] 검증 → [N] **CI 그린 확정** + 머지. 사용자 PIE 차후([[feedback-autonomous-slices]]). 머지 전 CI 별도 확인([[feedback-ci-before-merge]], [[reference-ci-retrigger]]). **Codex 커밋/푸시 누락 PM 보완**([[reference-codex-invoke]]).

## 6. 리스크
| 리스크 | 완화 |
| --- | --- |
| 1회차 보상 변경으로 기존 회귀 | RebirthCount 0·Lv100 = 5(기존과 동일) 보장 + 회귀 테스트 |
| 보상 폭주(무한 가속) | 선형 계수(횟수×2 + 레벨/10) 보수적 + balance 점검(prestige 곡선) |
| Preview/실제 보상 불일치 | 동일 GetRebirthPointsReward 사용(Preview=Rebirth 직전 값), 일치 테스트 |
| 서버↔클라 parity | RebirthFormula DefinitionParity(정수, 라운딩 일치) |
| 기존 환생 리셋 회귀 | Rebirth() 리셋 로직 보존, 보상 계산만 교체 |

## 7. 후속
- 환생 토큰/전용 상점, 초월(환생 위 prestige 단계), 환생 보너스 종류 확장, 서버 권위 환생.
