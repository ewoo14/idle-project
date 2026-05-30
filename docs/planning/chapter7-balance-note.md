# 챕터7 밸런스 노트

> 대상: 챕터7 슬라이스 · 작성 2026-05-30 · 스펙 [`2026-05-30-chapter7-design.md`](../superpowers/specs/2026-05-30-chapter7-design.md)
> **세이브 무변경**(멀티챕터 일반화 #66/#93, 글로벌 idx, SaveVer 25).

## 1. 스테이지 (글로벌 61~70)
- 적 스탯/보상 = ch6(51~60) 연속 곡선 외삽(`1+(globalIdx-1)*0.15` 연장, 급점프 없음).
- 7-5(글로벌 65)=미니보스, 7-10(글로벌 70)=챕터보스 — IsElite(X-5)/IsBoss(X-10) 자동. TotalChapters 6→7.

## 2. 약점 (61~70, Dark 가중, None 없음)
61 Dark/62 Fire/63 Holy/64 Lightning/65 Dark/66 Ice/67 Dark/68 Fire/69 Holy/70 Dark → Dark 4·5속성 전부. 7-5/7-10 Dark(ch6 패턴), default None 미발생(#70).

## 3. 메인 퀘스트 (ch7, 6개 체인)
main_ch7_001~006: kill(140)→clear→reach_level(85)→climb_tower(50, 7-5)→kill(145)→defeat_boss(1, 7-10). 보상 gold 224k~428k/exp 168k~321k(ch6 연속). 선행 ch6 finale→ch7 순차.

## 4. 페이싱/세이브
- ch6 연속 곡선이라 median 큰 영향 없음. **세이브 무변경**(SaveVer 25, 글로벌 idx, #93 입증).
