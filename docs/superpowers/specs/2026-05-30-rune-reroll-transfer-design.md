# 룬 리롤 + 전송 (Rune Reroll & Transfer) — 설계 문서

> 작성일: 2026-05-30 · 작성: PM/Claude · 분류: 룬 시스템 확장 4(#61~64 후속) · 메타 심화
> 대상 PR: 단일 슬라이스 · 상태: PM 자율 진행(사용자 "PM 판단 후 진행, 질문 없음")

## 0. 한 줄 요약
보유 룬을 **리롤**(세트 재설정 / 등급 상승 시도)하고 강화 레벨을 **전송**(투자 보존)하는,
룬 정수·골드 소비형 메타 심화. 무한 성장(등급 상승 무한 sink) + QoL(전송).

## 1. 배경
- 룬 트랙(#61 장착/#62 도감/#63 직업룬/#64 세트) 완비. 메모리 명시 후속 후보 = "룬 리롤·전송".
- 룬 파워는 RuneType+Rarity+EnhanceLevel+Set에서 **결정적 산출**(인스턴스 랜덤 값 저장 없음) →
  리롤/전송은 기존 영속 필드(Rarity/RuneSet/EnhanceLevel)만 변경 → **SaveVer bump 불필요**.
- [[project_infinite_growth]]: 등급 상승 무한 시도(확률 곡선) = 룬 정수 sink. [[project_content_richness]].

## 2. 핵심 결정 (PM)
| 항목 | 결정 |
| --- | --- |
| 세트 리롤 | `ERuneSet`(Offense/Bastion/Vitality/Fortune) 랜덤 재설정. **룬 정수** 비용. 결과 균등 랜덤(현재와 같아도 소비). |
| 등급 상승 시도 | `Rarity` +1단계 **확률 시도**(성공=한 단계↑, 실패=유지·**영구 파괴 없음**). **룬 정수+골드**. Mythic(최고)은 불가. 성공률은 현재 등급↑일수록 ↓(무한 sink). |
| 룬 전송 | source 룬 `EnhanceLevel` → target 룬으로 이전(target = max(target, source) 또는 source 값으로 상향), **source 룬 소모(삭제)**. 룬 정수 비용(source 레벨 비례). 동일 룬/장착 룬 제약은 단순화(장착 룬도 허용하되 인덱스 유효성 검증). |
| RNG 권위 | **클라 권위**(로컬 세이브). 클라 RuneFormula가 비용/확률 산출+RNG, 서버 `rune.ts`는 **parity 미러**(비용/확률 함수, 테스트용) — #71 ResolveAttempt/#63 패턴. |
| 세이브 | **변경 없음**(기존 FRuneSaveEntry 필드만 변경) |

## 3. 공식 (초기값 — balance-note 확정)
```
# 세트 리롤
RerollSetEssenceCost(rarity) = REROLL_SET_BASE * (rarity 가중)         # 룬 정수
결과: Offense/Bastion/Vitality/Fortune 중 균등 랜덤

# 등급 상승 시도 (Rarity r → r+1, r < Mythic)
UpgradeEssenceCost(r), UpgradeGoldCost(r)  = 기하 증가
UpgradeSuccessChance(r) = clamp(BASE_CHANCE * DECAY^r, 최소~최대)      # 등급↑ 확률↓
성공: Rarity = r+1 / 실패: 변화 없음(자원만 소모)

# 룬 전송
TransferEssenceCost(sourceLevel) = TRANSFER_BASE + sourceLevel * STEP
효과: target.EnhanceLevel = max(target.EnhanceLevel, source.EnhanceLevel); source 삭제
```

## 4. 통합 지점 (5-team)
| 파트 | 작업 |
| --- | --- |
| backend | `rune.ts` 확장: `getRerollSetEssenceCost`/`getRarityUpgradeEssenceCost`/`getRarityUpgradeGoldCost`/`getRarityUpgradeChance`/`getTransferEssenceCost`(클라 parity). vitest. |
| character | `RuneFormula` 미러 함수 + `RuneService::RerollRuneSet(idx)`/`TryUpgradeRuneRarity(idx, out)`/`TransferEnhancement(srcIdx, dstIdx)`(룬 정수/골드 차감은 GameInstance 경유), RNG 클라. Automation(비용 parity·전송 결과·등급 상한·실패 시 불변). |
| designer | 룬 패널: 선택 룬에 리롤(세트)·등급 상승 시도·전송 CTA + 비용/확률 표시 + ko/en. 표준 jumbo. |
| balance | `docs/planning/rune-reroll-transfer-balance-note.md`: 비용·확률 곡선, 무한 sink 분석, 세이브 무변경. |
| qa | 세트 리롤(소비·랜덤), 등급 상승(성공/실패/상한/자원부족 거부), 전송(레벨 이전·source 삭제·자원), parity. jumbo+ue-automation 게이트. |

## 5. 스코프
**In:** 세트 리롤 + 등급 상승 시도 + 전송 + parity + UI + balance/qa. 세이브 무변경.
**Out:** RuneType 리롤(슬롯 정체성 혼란 회피), 룬 정수 외 신규 재화, 직업룬 등급 상승(동일 패턴 후속).

## 6. 리스크
| 리스크 | 완화 |
| --- | --- |
| RNG parity drift(#61 교훈) | 클라 권위 RNG + 서버 비용/확률 **함수 parity** 테스트. 등급 상한/실패 시 불변 단언. |
| 등급 상승 파워크리프 | 확률 곡선(등급↑ 성공률↓) + 골드+정수 이중 비용으로 페이싱. balance median 재측정. |
| 전송 후 source 잔존/인덱스 꼬임 | source 삭제 후 인덱스 재정렬, 장착 슬롯 참조 갱신 검증. |
