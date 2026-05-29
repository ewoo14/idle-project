# 룬 리롤 + 전송 밸런스 노트 (룬 시스템 확장4)

> 대상: 룬 리롤/전송 슬라이스 · 작성 2026-05-30 · 스펙 [`2026-05-30-rune-reroll-transfer-design.md`](../superpowers/specs/2026-05-30-rune-reroll-transfer-design.md)
> **세이브 변경 없음**(룬 파워 결정적 산출 — 기존 FRuneInstance 필드 Rarity/RuneSet/EnhanceLevel만 변경).

## 1. 세트 리롤 (룬 정수)

`GetRerollSetEssenceCost(rarity)` 레어도별: Rare 20 / Epic 50 / Unique 100 / Legendary 200 / Transcendent 400 / (Mythic전) 800 / Mythic 1500.
- ERuneSet(Offense/Bastion/Vitality/Fortune) 균등 랜덤 재설정(현재와 같아도 소비). 세트 효과(#64) 맞춤용.

## 2. 등급 상승 시도 (룬 정수 + 골드, 확률, 무한 sink)

| rarity(r) | 정수 | 골드 | 성공률 |
| --- | --- | --- | --- |
| Rare(1) | 100 | 5,000 | 0.60 |
| Epic(2) | 250 | 15,000 | 0.45 |
| Unique(3) | 600 | 50,000 | 0.30 |
| Legendary(4) | 1,500 | 150,000 | 0.20 |
| Transcendent(5) | 4,000 | 500,000 | 0.12 |
| (→Mythic)(6) | 10,000 | 1,500,000 | 0.05 |
| Mythic(7) | — | — | 0 (불가) |

- 성공 = Rarity +1단계, 실패 = 변화 없음(자원만 소모, **영구 파괴 없음**). 등급↑일수록 비용↑·확률↓ = 무한 정수/골드 sink([[project_infinite_growth]]).
- 확률은 서버 `Math.fround` ↔ 클라 `float` 1:1 parity([[reference_ue_headless_verify]] §RNG, #61 교훈). RNG는 클라 권위(로컬 세이브), 서버는 비용/확률 함수 미러(테스트).

## 3. 룬 전송 (룬 정수)

`GetTransferEssenceCost(sourceLevel) = 50 + max(0, sourceLevel) * 25` (Lv0=50, Lv10=300).
- 효과: `Dst.EnhanceLevel = max(Dst, Src)`, **Src 룬 소모(삭제)**. 강화 투자 보존 QoL — 좋은 base 룬으로 갈아탈 때 강화 레벨 이전.
- Src 삭제 후 장착 슬롯 인덱스 보정(밀린 인덱스 감소, src 슬롯 해제). (구현 중 EnsureSlotCount 순서 버그 1건 PM 적발·수정.)

## 4. 경제/페이싱

- 등급 상승은 골드+정수 이중 비용 + 확률 곡선으로 파워크리프 억제. 룬 정수는 분해(#61)·던전 Essence·길드 상점(G2)·길드 보스(G3)에서 공급 → sink 다변화로 건강한 순환.
- median 페이싱 영향: 등급 상승은 선택적 가속(필수 아님) → 기존 5.328h 곡선 크게 흔들지 않음. 통합 검증에서 재확인.
- 수치는 초기값 — 룬 정수 수급/등급 분포 데이터로 재튜닝.

## 5. 세이브

- **변경 없음**(SaveVer 19 유지). 기존 영속 필드만 변경 → 마이그레이션 불필요.
