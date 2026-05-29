# 칭호(Title) 시스템 QA 검증 노트

> 작성 2026-05-30 · 검증: PM · SaveVer 20→21

## 1. 자동 검증 게이트

| 게이트 | 범위 | 결과 |
| --- | --- | --- |
| 서버 vitest (`src/core/formulas`) | title 카탈로그/해금/보너스 parity | **title 12/0** (전체 585/0) |
| 서버 lint/build | 전체 | **GREEN** |
| UE jumbo 빌드 | 전체(ODR) | **Succeeded** |
| UE Automation(클라) | Title(해금/장착/보너스 E2E)/GameCore(SaveSystem v21)/Combat/Achievement | **119/0** |
| UE Automation(UI) | Title/Localization(CsvIntegrity 44키)/UI | **36/0** |

## 2. 시나리오 커버리지

- **해금**: 업적 메트릭 `GetMetricValue >= threshold` 충족 시 자동 해금(영구). 경계(threshold-1 미해금/threshold 해금).
- **장착**: 해금된 칭호만, 1개만(EquippedTitleId 단일). 미해금 장착 거부.
- **보너스 적용**: AllStatPct(CP 반영)/GoldPct(+15% 골드 1회)/ExpPct/CritDmgPct — 각 단일 지점, 장착 1개. 해제 시 원복(이중 적용 없음).
- **parity**: 클라 TitleService 카탈로그 ↔ 서버 title.ts 18종 1:1(임계/보너스 비율).
- **세이브 v21**: UnlockedTitleIds/EquippedTitleId 라운드트립, SaveVer<21=빈(회귀안전).

## 3. PM 적발·수정 (테스트 버그)

- BonusApplicationE2E가 base CP를 **업적 메트릭 기록 전**에 측정 → 칭호 해금용 `RecordAchievementMetric(MonstersKilled, 10000)`이 **업적(AchievementMultiplier)도 해금**해 해제 후에도 CP가 안 돌아오던 오탐 → base를 메트릭 기록 **후** 측정하도록 수정(업적 보너스와 칭호 효과 분리). UE Automation이 적발 → fix 후 GREEN. (구현은 정상.)

## 4. 한계 / 후속

- 칭호 복수 장착(슬롯 확장)·칭호 합성/강화·칭호 전용 신규 메트릭은 후속.
- 보너스 수치/임계 초기값 재튜닝. PR 본문 칭호 패널 스크린샷.
