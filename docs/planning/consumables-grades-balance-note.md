# 소비 아이템 등급별 차등 (V1.5) — 밸런스 노트 (PR #81)

> PM/Claude 직접 작성(Codex 한도 대체). #73 소비에 등급(소/중/대) 차원 추가.

## 1. 등급 % 모델
- 지속시간은 등급 무관 타입별 고정(1800초=30분). **효과 %만 차등**:
  - **Lesser(소) = Standard × 0.5**, **Standard(중) = #73 기존값**, **Greater(대) = Standard × 2.0**.
- 예시:

| 타입 | Lesser | Standard | Greater |
| --- | --- | --- | --- |
| AttackTonic(공격) | +15% | +30% | +60% |
| GuardTonic(수호) | +15% | +30% | +60% |
| AllStatElixir(만능) | +10% | +20% | +40% |
| FortuneScroll(행운/드롭) | +15% | +30% | +60% |
| GoldFeast(황금/골드) | +25% | +50% | +100% |
| WisdomBooster(지혜/EXP) | +25% | +50% | +100% |

## 2. 수급 / 가격
- 골드 상점 3등급 판매(기어롤 비용 #38 곡선 기준): **Lesser ×0.6 / Standard ×1.0 / Greater ×2.5**. Greater는 효과 2배에 가격 2.5배 → 약간 비효율(저장가치 vs 즉시성 트레이드오프).
- 드롭은 grade-aware `AddConsumable` API 제공(실수급 가중은 향후 드롭 경로 연결 시 Lesser 위주 배분 — 본 슬라이스는 상점 수급 중심).

## 3. 무한 성장 비침범
- 소비 버프는 **시간제·소모형**(영구 성장 직교) — 등급이 올라도 30분 한정. Greater(대)도 영구 누적 아님.
- balance-sim 코어 환생 페이싱 무관: median **5.328h**(inside-target) 무변동.

## 4. 비고
- SaveVersion 15→16(v15 엔트리=Standard 마이그레이션). 등급별 지속시간 차등은 범위 외(V1.5는 % 만).
