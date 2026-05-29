# 장비 잠재 V2 밸런스 노트 (#71 후속)

> 대상: 잠재 V2 슬라이스 · 작성 2026-05-30 · 스펙 [`2026-05-30-potential-v2-design.md`](../superpowers/specs/2026-05-30-potential-v2-design.md)
> **세이브 무변경**(SaveVer 22 유지, enum 확장 + PotentialLine4 태그드 프로퍼티 전방호환).

## 1. Transcendent 등급 (5번째)

| 등급 | 줄 수 | 값 범위(기본) |
| --- | --- | --- |
| Rare | 1 | (기존) |
| Epic | 2 | (기존) |
| Unique | 3 | (기존) |
| Legendary | 3 | [0.10, 0.15] |
| **Transcendent** | **4** | **[0.13, 0.195]** (Legendary ×1.3) |

- 아이템 등급 상한: Rare→Epic / Epic→Unique / Unique→Legendary / **Legendary·Transcendent·Mythic 아이템 → Transcendent 허용**.
- Rank 큐브 등급 상승: 기존 ...→Legendary 0.08, **Legendary→Transcendent 0.05**(낮은 확률 무한 chase).

## 2. 신규 옵션 3종 (값 = 등급 기본범위 × 배수)

| 옵션 | 배수 | 적용 지점 |
| --- | --- | --- |
| AllStatPercent | ×0.4 (보수적, 전 스탯) | RefreshDerivedStats 전 스탯 배수(마스터리/펫/칭호 옆) |
| GoldFindPercent | ×1.5 | AddGold 골드 배수(펫/칭호/길드 옆) |
| DropRatePercent | ×1.5 | 펫 Drop(#69) 집계점 합류 |

- 전투 8종(PhysAtk~CritDmg %)은 기존 경로 불변. 신규 3종은 **각 단일 집계 지점**(이중 적용 가드 #72).
- 3자리 라운딩(`round(x*1000)/1000`, fround 아님) 서버↔클라 1:1. RNG 클라 권위(#71).

## 3. 경제/페이싱

- Transcendent 도달은 Rank 큐브 0.05 확률 → 골드/큐브 무한 sink. 값 ×1.3 상향은 보수적.
- AllStat ×0.4는 전 스탯 곱이라 의도적 소(파워크리프 억제). Gold/Drop ×1.5는 경제 옵션으로 체감.
- median 영향: 통합 검증 재측정. 수치 초기값 재튜닝 대상.

## 4. 세이브

- **무변경**(22 유지). 기존 줄(0~8 stat) 불변, 신규 값(9~11)·등급5·4번째 줄은 신규 롤만 → 전방호환. 기존 v22 세이브 라운드트립 회귀 테스트 통과.
