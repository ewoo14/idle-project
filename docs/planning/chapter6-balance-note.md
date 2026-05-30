# 챕터6 밸런스 노트

> 대상: 챕터6 슬라이스 · 작성 2026-05-30 · 스펙 [`2026-05-30-chapter6-design.md`](../superpowers/specs/2026-05-30-chapter6-design.md)
> **세이브 무변경**(멀티챕터 일반화 #66/#80, 글로벌 idx).

## 1. 스테이지 (글로벌 51~60)

- 적 스탯/보상 = ch5(41~50) **연속 곡선 외삽**(`1 + (globalIdx-1)*0.15` 등 기존 공식 그대로 연장, 급점프 없음).
- 6-5(글로벌 55)=미니보스, 6-10(글로벌 60)=챕터보스 — 기존 IsEliteStage(X-5)/IsBossStage(X-10) 자동.
- TotalChapters 5→6, StagesPerChapter 10 불변.

## 2. 약점 (51~60, Dark 가중, None 없음)

51 Dark / 52 Lightning / 53 Ice / 54 Holy / 55 Dark / 56 Fire / 57 Dark / 58 Ice / 59 Holy / 60 Dark
→ Dark 4(51/55/57/60)·Ice 2·Holy 2·Lightning 1·Fire 1. ch5 패턴(6-5/6-10 Dark), default None 미발생(#70 교훈).

## 3. 메인 퀘스트 (ch6, 6개 체인)

main_ch6_001~006: kill(125)→clear(1)→reach_level(75)→climb_tower(45, 6-5)→kill(130)→defeat_boss(1, 6-10).
보상 gold 105k~200k / exp 79k~150k (ch5 88k/66k에서 ~1.13~1.2× 연속). 선행 체인(ch5 finale→ch6 순차).

## 4. 페이싱/경제

- ch5 연속 곡선이라 median 5.328h 큰 영향 없음(검증된 스케일). 통합 검증 재측정.
- **비고**: `tools/balance-sim/index.ts`의 `TOTAL_CHAPTERS=4`는 독립 시뮬 픽스처로 ch5도 미반영 — 본 슬라이스 범위 밖, 시뮬 갱신은 별도 후속.

## 5. 세이브

- **무변경**(SaveVer 22 유지). 진행도 글로벌 idx 기반, 마이그레이션 불필요(#80 입증).
