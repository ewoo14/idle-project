# 일일/주간 미션 QA 검증 노트

> 작성 2026-05-30 · 검증: PM · SaveVer 21→22

## 1. 자동 검증 게이트

| 게이트 | 범위 | 결과 |
| --- | --- | --- |
| 서버 vitest (`src/core/formulas`) | mission 카탈로그/보상 parity | **mission 9/0** (전체 594/0) |
| 서버 lint/build | 전체 | **GREEN** |
| UE jumbo 빌드 | 전체(ODR) | **Succeeded** |
| UE Automation(클라) | Mission(진행/완료/수령/일·주 리셋/후크/parity)/GameCore(SaveSystem v22)/Dungeon | **91/0** |
| UE Automation(UI) | Mission/Localization(CsvIntegrity 18키)/UI | **36/0** |

## 2. 시나리오 커버리지

- **진행**: RecordAchievementMetric 중앙 후크(5 메트릭) + 던전 실행(DungeonRuns) 단일 지점 누적. Maximum 모드 메트릭 미매핑(미션 영향 없음) 검증.
- **완료/수령**: IsComplete 경계(target-1/target), ClaimMission(완료 수령·미달 거부·중복 거부), 보상 단일 지급(gold/essence/consumable).
- **리셋**: 일일(UTC date 변경→daily progress/claimed 0·weekly 유지), 주간(ISO week 변경→weekly 0). 최초 마커 미설정 시 리셋 없이 초기화.
- **세이브 v22**: 진행/수령/마커 라운드트립, SaveVer<22=빈(회귀안전).
- **parity**: 클라 MissionService 카탈로그 ↔ 서버 mission.ts 10종 1:1(target/reward).

## 3. PM 적발·수정 (실버그)

- `GetMissionService() const`가 EnsureMissionService 미호출 → Init 미경유 컨텍스트(테스트)에서 첫 호출 시 null 반환 → ClaimReward/SaveRoundTrip E2E 조기 실패. **GetMissionService를 lazy-ensure(const_cast 표준 패턴)로 수정** — getter가 null 반환하지 않도록(인게임은 Init에서 ensure되나 견고성↑). UE Automation이 적발 → fix 후 GREEN.

## 4. 한계 / 후속

- 랜덤 미션 풀/새로고침·일일 로그인 보상·미션 패스는 후속(시즌 #22 별도).
- 보상/목표 수치 초기값 재튜닝. PR 본문 미션 패널 스크린샷.
