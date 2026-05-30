# 환생 특성 QA 검증 노트

> 작성 2026-05-30 · 검증: PM · SaveVer 23→24

## 1. 자동 검증 게이트

| 게이트 | 범위 | 결과 |
| --- | --- | --- |
| 서버 vitest (`src/core/formulas`) | rebirthPerk 포인트/보너스 parity | **rebirthPerk 10/0** (전체 617/0) |
| 서버 lint/build | 전체 | **GREEN** |
| UE jumbo 빌드 | 전체(ODR) | **Succeeded** |
| UE Automation(클라) | RebirthPerk/Rebirth/GameCore(SaveSystem v24)/Combat | **135/0** |
| UE Automation(UI) | RebirthPerk/Localization(CsvIntegrity 10키)/UI | **42/0** |

## 2. 시나리오 커버리지

- **분배**: AllocatePerk(Available 한도·None 거부·중복), DeallocatePerk(level>0), ResetPerks(무료 재분배). Available = Total - Spent ≥ 0.
- **포인트 출처**: GetTotalPoints = rebirthCount(1/환생), 환생 시 SetTotal·초월 시 Reset.
- **보너스 적용**: 6종 각 단일 지점(Gold/Drop/CritDmg/AllStat/Exp/Offline %) — CP/재화 반영, 이중 적용 없음.
- **비파괴**: 기존 RebirthBonusPoints(HP/공격) 불변 회귀 검증.
- **parity**: 클라 RebirthPerkService ↔ 서버 rebirthPerk.ts PerkStep/Total(f32 fround) 1:1.
- **세이브 v24**: Allocations(enum-key TMap) 라운드트립, SaveVer<24=빈(회귀안전).

## 3. 후속/비고

- getter lazy-ensure(#91 교훈) 적용. enum-key TMap UPROPERTY 직렬화 게이트 검증 통과.
- 특성 노드 트리(선후행)·특성 RNG·초월 특성은 후속. 수치 재튜닝. PR 본문 환생 특성 패널 스크린샷.
