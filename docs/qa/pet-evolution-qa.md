# 펫 진화(별/Star) QA 검증 노트 (펫 시스템 심화)

> 작성 2026-05-30 · 검증: PM · SaveVer 19→20

## 1. 자동 검증 게이트

| 게이트 | 범위 | 결과 |
| --- | --- | --- |
| 서버 vitest (`src/core/formulas`) | petBonus 진화 비용/별 배수 parity | **petBonus 10/0** (전체 573/0) |
| 서버 lint/build | 전체 | **GREEN** |
| UE jumbo 빌드 | 전체(ODR) | **Succeeded** |
| UE Automation(클라) | Pet(진화/별 배수/parity)/GameCore(SaveSystem v20)/Combat | **113/0** |
| UE Automation(UI) | Pet/Localization(CsvIntegrity)/UI | **36/0** |

## 2. 시나리오 커버리지

- **진화**: 골드 차감(GetPetEvolveCost 현재 별) + star++(확정 성공, RNG 없음). 미보유 펫 거부, 골드 부족 거부.
- **별 배수**: GetPetStarMultiplier(별) — star0=1.0/선형 무한. 장착 펫 보너스에 단일 적용(레벨 배수와 직교 — 같은 레벨 고정 후 별만 올려 보너스 증가 확인). 비장착 펫 별 변화는 무영향.
- **parity**: 클라 GetPetEvolveCost/GetPetStarMultiplier ↔ 서버 petBonus.ts 1:1(비용 floor 정수, 배수 fround float 순서 일치).
- **세이브 v20**: PetStars 라운드트립 보존, SaveVer<20 입력=0성(회귀안전).

## 3. 한계 / 후속

- 펫 보너스는 스탯 %(Gold/Drop/Exp 퍼센트엔 별 배수 미적용 — 스탯 보너스 한정, 설계대로).
- 펫 합성/중복(OwnedPetIds Set이라 중복 미추적)·별 RNG·펫별 차등 진화 곡선은 후속.
- 수치 초기값 재튜닝. PR 본문 펫 패널(별·진화 버튼) 스크린샷.
