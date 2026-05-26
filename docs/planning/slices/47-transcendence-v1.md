# PR #47 기획서 — 초월 Transcendence (무한 2차 prestige)

> 무한 성장([[project-infinite-growth]]) 2차 prestige. 환생(#19/#46)은 영구 포인트를 쌓지만 단일 층위다. **초월**을 추가 — 환생을 일정 횟수 이상 한 뒤 초월하면, 환생 진행을 더 깊게 리셋하는 대신 **영구 글로벌 스탯 배수**(무한 누적)를 얻는다. 환생 위의 2차 prestige 층으로 장기 무한 성장 목표를 제공한다. client+server+UI+balance+qa 5-team.

## 1. 목표 / DoD
환생을 N회 이상 한 뒤 초월하면 초월 횟수가 늘고, 영구 글로벌 스탯 배수가 커져 모든 전투 스탯이 강해진다.

### DoD 검증
1. CanTranscend: RebirthCount ≥ TranscendRebirthThreshold(예 5). 초월 시 TranscendCount++, 환생 관련(RebirthCount/RebirthBonusPoints)·레벨·분배·스테이지 등 **깊은 리셋**.
2. 글로벌 배수: GetTranscendStatMultiplier(TranscendCount) = 1 + count×0.25 (무한). RefreshDerivedStats 가 Derived 의 Hp/PhysAtk/MagicAtk/PhysDef/MagicDef 에 곱(rate 류 크리/공속 제외, 클램프 보존).
3. 초월 0회 = 배수 1.0 → 기존 스탯 불변(회귀 안전).
4. HUD 초월 패널(상태/요구 환생수/현재 배수/다음 배수/초월 버튼). 서버 TranscendFormula 미러 + parity. 서버 Vitest + UE 빌드/Automation GREEN.

## 2. 범위 (In Scope)
### 2.1 초월 공식 (메인, C++)
- `TranscendFormula.h/.cpp`(GameCore/, 순수 static): `TranscendRebirthThreshold`(예 5), `GetTranscendStatMultiplier(int32 TranscendCount)` = 1 + max(0,count)×0.25 (무한, float). `CanTranscend(int32 RebirthCount)` = RebirthCount ≥ Threshold.
### 2.2 초월 상태/행위 (메인, C++)
- `UIdleGameInstance`: `int32 TranscendCount`. `bool CanTranscend() const`(RebirthCount 기준). `bool Transcend()`: CanTranscend 시 ++TranscendCount + 깊은 리셋(RebirthCount=0, RebirthBonusPoints=0, CharacterLevel=1, 분배 리셋, Gold(초월은 골드 보존/리셋 정책 — V1 0 또는 환생처럼 일부), bChapter1BossDefeated=false, StageService 리셋). `GetTranscendCount()`/`GetTranscendStatMultiplier()`/`PreviewTranscendMultiplier()`. 델리게이트(OnTranscend) + 기존 broadcast.
- `AIdleCharacter::RefreshDerivedStats`: DeriveStats 결과에 `× GameInstance->GetTranscendStatMultiplier()` 를 Hp/PhysAtk/MagicAtk/PhysDef/MagicDef 에 적용(rate 류 제외). count 0 → ×1.0 불변.
### 2.3 UI (디자이너)
- HUD 초월 패널: 상태(가능/불가)·요구 환생수(현재/threshold)·현재 배수·초월 후 배수·초월 버튼(HitBox). 로컬라이즈 ko/en.
### 2.4 서버 (백엔드)
- `server/src/core/formulas/transcend.ts`: getTranscendStatMultiplier(count)(Math.fround float parity)/canTranscend(rebirthCount)/TRANSCEND_REBIRTH_THRESHOLD 미러 + parity 테스트.
### 2.5 데이터/밸런스
- threshold(환생 5회)·배수(+25%/초월) + 무한 prestige 곡선(환생×초월 복합 성장) + 문서.
### 2.6 테스트
- 서버 Vitest(배수/임계/미러/parity) + 클라 Automation(CanTranscend 임계, Transcend 리셋+count++, 배수 스탯 반영, count 0 회귀, Preview 일치).

## 3. 범위 외
- 초월 전용 재화/상점/스킬트리(후속), 초월 위 3차 prestige, 초월 시 환생 보너스 영구 보존(V1 깊은 리셋).
- 서버 권위 초월(클라 권위 V1).

## 4. 작업 분배 — Codex 호출
| 파트 | 작업 | 비중 |
| --- | --- | --- |
| 캐릭터·전투 (메인) | TranscendFormula + GameInstance 초월 상태/행위 + RefreshDerivedStats 배수 + Automation | ✅ 메인 (`character`) |
| 백엔드 | transcend.ts 미러 + parity | ✅ 보조 (`backend`) |
| 디자이너 | 초월 패널 HUD + 로컬라이즈 | ✅ 보조 (`designer`) |
| 밸런스 | threshold/배수 + prestige 곡선 + 문서 | ✅ 보조 (`balance`) |
| QA | 초월 임계/리셋/배수/회귀 시나리오 | ✅ 보조 (`qa`) |

## 5. 워크플로우 v3
[1] 기획+PR → [2] Codex character 메인(+backend/designer/balance/qa) → [3] Claude TM → [4] Codex TM+fix → [5] 검증 → [N] **CI 그린 확정** + 머지. 사용자 PIE 차후([[feedback-autonomous-slices]]). 머지 전 CI 별도 확인([[feedback-ci-before-merge]], [[reference-ci-retrigger]]). **Codex 커밋/푸시 누락 PM 보완**([[reference-codex-invoke]]).

## 6. 리스크
| 리스크 | 완화 |
| --- | --- |
| 초월 깊은 리셋이 진행 손실 과대 | V1 배수가 그 손실을 보상(+25%/초월), threshold(환생 5) + balance 점검. 리셋 범위 명확 |
| 글로벌 배수 적용 위치(중복/누락) | RefreshDerivedStats 단일 지점에서 Derived 곱, rate 류 제외 + 클램프 보존. count 0 ×1.0 회귀 테스트 |
| 서버↔클라 배수 parity | TranscendFormula Math.fround float parity + 경계 테스트 |
| 환생(#46)과 상호작용 | 초월이 RebirthCount/RebirthBonusPoints 리셋 — 환생 보상 스케일(count 기반)과 정합(초월 후 환생 0부터) |

## 7. 후속
- 초월 전용 재화/상점/스킬트리, 3차 prestige, 초월 보너스 종류 확장, 서버 권위 초월.
