# 펫 진화 (Pet Evolution / Star) — 설계 문서

> 작성일: 2026-05-30 · 작성: PM/Claude · 분류: 펫 시스템 심화(#69 후속) · 무한 성장 메타
> 대상 PR: 단일 슬라이스 · 상태: PM 자율(사용자 "PM 판단 후 진행, 질문 없음")

## 0. 한 줄 요약
보유 펫을 **별(Star) 무한 등급**으로 진화시켜 장착 펫 보너스에 배수를 더하는 골드 소비형
무한 성장 축. 펫 레벨(#42 FeedPet)과 **직교**(레벨=골드 점진 성장 / 별=골드 기하 진화).

## 1. 배경
- 펫(#69): 10종 + 8 보너스 타입 + 레벨(FeedPet). 장착 펫 1마리 보너스만 적용(GetEquippedPetStatBonus).
- 성장 천장 부재 원칙([[project_infinite_growth]], "낮은 캡 지양") → 별은 **무한**(기하 비용).
- [[project_content_richness]]: 펫에 진화 레이어 추가로 장기 목표 부여.

## 2. 핵심 결정 (PM)
| 항목 | 결정 |
| --- | --- |
| 별 등급 | **무한**(0성 시작), 펫별 독립(`PetStars: TMap<FString,int32>`) |
| 진화 비용 | **골드 기하 증가**(`PET_EVOLVE_BASE * GROWTH^star`) — 무한 sink, 신규 재화 없음 |
| 진화 판정 | **확정 성공(RNG 없음)** — 깔끔·parity 단순, 무한성은 비용 곡선으로 |
| 별 효과 | 장착 펫 보너스 × `(1 + PET_STAR_STEP * star)` (선형 무한). **레벨과 직교**(레벨 배수 × 별 배수 분리 곱) |
| 적용 지점 | `GetEquippedPetStatBonus` — 장착 펫의 별만 반영(이중 적용 금지) |
| 세이브 | **19→20**(PetStars 맵, 누락=0성 회귀안전) |

## 3. 공식 (초기값 — balance-note 확정)
```
PetEvolveCost(star) = floor(PET_EVOLVE_BASE * PET_EVOLVE_GROWTH^star)   # 골드, 0성→1성부터
PetStarMultiplier(star) = 1 + PET_STAR_STEP * star                     # 선형 무한 (예 STEP=0.15 → 5성 ×1.75)
GetEquippedPetStatBonus = (펫 정의 BonusPercent × 레벨배수) × PetStarMultiplier(장착펫 별)
EvolvePet(petId): owned && gold >= PetEvolveCost(현재 별) → 골드 차감, star++
```

## 4. 통합 지점 (5-team)
| 파트 | 작업 |
| --- | --- |
| backend | `petBonus.ts` 확장: `getPetEvolveCost(star)`/`getPetStarMultiplier(star)`(클라 parity 상수 PET_EVOLVE_BASE/GROWTH/PET_STAR_STEP). vitest. |
| character | `PetLevelFormula`(또는 PetService) 미러 함수 + `PetService`: `PetStars` 맵·`GetPetStar(id)`·`EvolvePet(id)`(골드 차감은 GameInstance 경유)·GetEquippedPetStatBonus에 별 배수 적용. SaveVer **19→20**(PetStars save/restore, 전 세이브 테스트 단언 갱신). GameInstance 진입점(골드 검증·차감·RefreshDerivedStats·Autosave). Automation(진화 비용 parity·별 배수·골드 차감·CP 반영·세이브 v20·미보유 거부). |
| designer | 펫 패널: 선택/장착 펫 별 표시 + 진화 버튼(비용·다음 별 효과) + ko/en + CsvIntegrity. 표준 jumbo. |
| balance | `docs/planning/pet-evolution-balance-note.md`: 진화 비용 곡선·별 배수·레벨과 직교·median 영향. |
| qa | 진화(골드 차감·별↑)·미보유/골드부족 거부·별 배수 CP 반영·세이브 v20·parity. jumbo+ue-automation 게이트. |

## 5. 스코프
**In:** 펫 별 무한 진화(골드) + 별 배수(장착) + UI + parity + SaveVer 20. **Out:** 펫 합성/중복(OwnedPetIds Set이라 중복 미추적), 별 RNG, 펫별 차등 진화(균일 곡선 V1).

## 6. 리스크
| 리스크 | 완화 |
| --- | --- |
| 레벨×별 이중 계산 | 별 배수는 GetEquippedPetStatBonus 최종 1곳, 장착 펫만(#72 교훈). |
| 파워크리프(무한 별) | 선형 배수 + 기하 골드 비용으로 페이싱, median 재측정. |
| 세이브 마이그레이션 | PetStars 누락=0성(회귀안전), SaveVer<20 가드. |
| parity drift | getPetEvolveCost/getPetStarMultiplier 서버↔클라 1:1(확정 성공이라 RNG 없음). |
