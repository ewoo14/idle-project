# 환생 특성 밸런스 노트

> 대상: 환생 특성 슬라이스 · 작성 2026-05-30 · 스펙 [`2026-05-30-rebirth-perks-design.md`](../superpowers/specs/2026-05-30-rebirth-perks-design.md) · SaveVer 23→24

## 1. 특성 포인트 출처

- `getTotalRebirthPerkPoints(rebirthCount) = max(0, rebirthCount)` — **1 / 환생**(무한 누적).
- **기존 RebirthBonusPoints(점당 HP+10/공격+2) 불변**(비파괴) — 환생 특성은 완전 별도 풀.
- 초월(Transcend) 시 환생 특성 리셋 + 포인트 0(rebirthCount 리셋 연동).

## 2. 6종 특성 (레벨당 % 비율, 선형 무한)

| 특성 | PerkStep(레벨당) |
| --- | --- |
| GoldPct (골드 획득) | +2% |
| DropPct (드롭률) | +2% |
| CritDmgPct (크리티컬 데미지) | +3% |
| AllStatPct (전체 스탯) | +1% |
| ExpPct (경험치) | +2% |
| OfflineEffPct (오프라인 효율) | +3% |

- `PerkBonus(perk, level) = PerkStep × level`(f32 fround 서버 parity). 무한(캡 없음).
- **자유 분배**: Σ Allocations ≤ Total, Available>0만 할당, 무료 리셋/재분배.
- 각 특성 **단일 적용 지점**(Gold→AddGold·Drop→펫 Drop 집계·CritDmg→크리뎀·AllStat→derived·Exp→AddExp·Offline→오프라인 보상). 기존 마스터리/펫/칭호/잠재 옆 합산(이중 적용 가드 #72).

## 3. 경제/페이싱

- AllStat ×1%는 전 스탯 곱이라 보수적(파워크리프 억제). Gold/Drop/Exp ×2%, CritDmg/Offline ×3%로 빌드 선택지.
- 포인트가 rebirthCount 1/환생이라 초반 분배는 소수 → 환생 누적에 비례한 점진 성장(무한). 빌드 다양성(드롭형/경제형/딜형) 부여.
- median 영향: 환생 깊이에 비례, 통합 검증 재측정. 수치 초기값 재튜닝.

## 4. 세이브

- **23 → 24**: `RebirthPerkAllocations`(TMap<ERebirthPerk,int32>). 총 포인트는 RebirthCount 파생(미저장). 누락=빈(회귀안전), SaveVer<24 가드.
