# 소비 아이템 + 임시 버프 (Consumables & Timed Buffs) — 설계 문서

> 작성일: 2026-05-29 · 대상 슬라이스: PR #73 · 브랜치: `plan/73-consumables`
> 분류: 신규 콘텐츠 카테고리 (미구현 "소비 아이템" 도입) · PM 자율 진행

---

## 0. 한 줄 요약

장비와 별개인 **소비 아이템**(물약/요리/주문서/부스터)을 신설한다. 사용하면
일정 시간 동안 능력치·획득량 **임시 버프**가 활성화된다. 드롭·던전·골드 상점·
퀘스트로 수급하며, 방치형을 해치지 않는 **선택적 부스터** 텍스처를 더한다.

---

## 1. 목적 / 배경

- GDD 전반에서 모든 시스템이 "소비 제외"로 명시 — **소비 아이템은 유일한
  미구현 핵심 카테고리**. 콘텐츠 풍부화([[project-content-richness]]) 직접 충족.
- 성장은 영구 시스템(장비/룬/펫/마스터리/환생)이 담당하므로, 소비 아이템은
  **시간제 부스터**로 차별화(영구 성장과 직교, 소모형).
- 방치형 정합: 사용은 **선택적**이며 미사용 시 기존 플레이 불변.

### 1.1 기존 시스템과의 관계

| 기존 | 소비 버프와의 관계 |
| --- | --- |
| 장비/룬/펫/마스터리/환생 | 영구·항상 적용 vs 소비 = 시간제·소모 (직교) |
| 강화 자원(보호서/큐브 등 #71) | 동일 "카운터형 자원" 저장 패턴 재사용, 단 소비는 *사용 시 버프 발동* |
| 상태이상(#30 CombatComponent) | 전투 내 DoT/슬로우 (적→나) vs 버프 = 계정 전역 시간제 (나에게 유리). 별도 경로 |

---

## 2. 핵심 결정 (PM 자율 확정)

| 항목 | 결정 | 근거 |
| --- | --- | --- |
| 형태 | **시간제 버프 부스터** (즉시 회복형 아님) | 자동전투라 HP 회복 의미 적음, 버프가 방치형에 적합 |
| 저장 모델 | 타입별 **보유 수량 카운터** + 활성 버프 **종료 유닉스타임** | #71 자원 카운터 + 기존 유닉스 시계 재사용 |
| 시간 기준 | **실시간 종료 타임스탬프**(오프라인에도 소진) | 단순·치트 저항. 오프라인 일시정지는 후속 |
| 스택 규칙 | 동일 타입 = 지속시간 **갱신/연장**, 다른 타입 = **동시 적용** | 명확·직관 |
| 수급 | 드롭·던전·골드 상점·퀘스트 (가챠 없음) | 기존 수급 경로 재사용 |

---

## 3. 소비 아이템 종류 (초기 6종)

| 타입(EConsumableType) | 효과 | 적용 경로 |
| --- | --- | --- |
| **AttackTonic** (공격 물약) | 물공+마공 +X% | RefreshDerivedStats 곱 |
| **GuardTonic** (수호 물약) | HP+물방+마방 +X% | RefreshDerivedStats 곱 |
| **AllStatElixir** (만능 비약) | 전 코어 스탯 +X% | RefreshDerivedStats 곱 |
| **FortuneScroll** (행운의 주문서) | 드롭률 +X% | 드롭 경로 가산 |
| **GoldFeast** (황금 요리) | 골드 획득 +X% | 골드 경로 곱 |
| **WisdomBooster** (지혜의 부스터) | EXP 획득 +X% | AddExp 경로 곱 |

- 등급/효과 강도는 단순 V1: 타입별 고정 % + 고정 지속시간(예: +30%, 30분).
  등급별 차등(소/중/대)은 후속.

---

## 4. 버프 적용 & 시간 모델

- **BuffService**(또는 GameInstance 보유 상태)가 타입별 활성 버프의 **종료
  유닉스타임**을 보관. `Now >= EndTime`이면 만료(0 기여).
- 스탯 버프: `GetBuffStatMultiplier()` → `RefreshDerivedStats`의 기존 곱
  (transcend×tower×achievement×mastery)에 **별도 항**으로 합류. 단, 타입별 적용
  대상이 다르므로:
  - AttackTonic → PhysAtk/MagicAtk 추가 곱
  - GuardTonic → Hp/PhysDef/MagicDef 추가 곱
  - AllStatElixir → 전 코어 곱
- 경제 버프: `GetGoldBuffPct()`/`GetExpBuffPct()`/`GetDropBuffAdd()` → 기존
  골드/EXP/드롭 경로(마스터리·펫과 동일 합류 지점).
- **이중 계산 방지**: 마스터리(#72) 교훈 — 각 버프 % 는 **단일 적용 지점**에서만.
  EXP는 `AddExp` 단일 경로, 골드/드롭은 처치/획득 경로 1회.

---

## 5. 수급 / 사용 흐름

1. **수급**: 몬스터 드롭(낮은 확률), 던전(#68) 보상, 골드 상점 구매(#38 곡선),
   퀘스트 보상 → 타입별 카운터 += N.
2. **사용**: 인벤토리에서 사용 → 카운터 -1, 해당 타입 버프 종료시각 =
   `Now + Duration`(이미 활성 시 연장). `RefreshPlayerCharacterStats()` 호출.
3. **만료**: 종료시각 경과 시 자동 0 기여(폴링/타이머 또는 조회 시점 판정).

---

## 6. 통합 지점 (구현 개요)

| 파트 | 작업 |
| --- | --- |
| character (메인) | `EConsumableType`, `FConsumableSaveEntry`, `FConsumableFormula`(%·지속), `UBuffService`(보유 수량·활성 버프·`GetBuffStatMultiplier`/경제 getter), GameInstance(보유/사용/수급 훅·RefreshDerivedStats 합류·세이브 v13→v14), 클라 Automation |
| backend | `consumable.ts` 미러(%·지속·parity) + `save.schema.ts` payload 선택 필드(보유/활성 버프) + vitest |
| designer | 소비 인벤토리 패널 + **활성 버프 바**(아이콘+남은 시간) + 사용 CTA + ko/en 로컬라이즈 + CsvIntegrity |
| balance | 버프 %·지속·수급률·상점 가격 + 무한 성장 비침범(영구 성장 대비 보조적) 문서 |
| qa | 시나리오(사용→버프→스탯/경제↑→만료→원복, 스택 규칙, 세이브 v13→v14, parity, 회귀) |

### 6.1 세이브
- SaveVersion **13→14**. 신규: 타입별 보유 수량, 타입별 활성 버프 종료 유닉스타임.
  v13 로드 시 보유 0·버프 비활성 마이그레이션. 클라우드 payload 선택 필드.

### 6.2 퀘스트(선택)
- `EQuestObjective::UseConsumable` 추가 가능(소비 사용 일일 퀘스트). V1 포함 여부는
  스코프에 따라 — 포함 시 quest 파트 경량 확장.

---

## 7. 스코프

**In Scope (V1):** 6종 소비 아이템 + 시간제 버프(스탯 3 + 경제 3), BuffService,
수급 4경로(드롭/던전/상점/퀘스트), 인벤토리+버프 바 UI, 세이브 v13→v14, 서버
미러+parity, balance 문서, 5-team.

**Out of Scope (후속):** 등급별 차등 효과(소/중/대), 즉시 회복형 소비(HP/MP),
오프라인 버프 일시정지, 자동 사용 옵션, 소비 제작(연금술).

---

## 8. 리스크

| 리스크 | 완화 |
| --- | --- |
| 버프 % 이중 적용 (마스터리 #72 교훈) | 각 % 단일 적용 지점 확정 + getter 소비처 추적 + qa 단조/원복 검증 |
| 시간제 버프 ↔ 세이브/오프라인 정합 | 종료 유닉스타임 절대값 저장(실시간 소진), 라운드트립 + 만료 경계 Automation |
| 영구 성장 잠식(버프가 과강) | 보조적 % + 지속 제한 + balance 문서, 무한 성장은 영구 시스템 담당 유지 |
| 신규 아이템 카테고리 = 인벤토리 혼선 | 장비(FItemInstance)와 **분리된 카운터 모델** — 장비 인벤 경로 불변(회귀안전) |
| 서버↔클라 drift | `consumable.ts` Math.fround 미러 + 경계 parity |

---

## 9. 워크플로우 v3 / 후속

[1] PM 기획+PR → [2] Codex 5-team → [3] Claude TM → [4] Codex TM+fix → [5] Claude 검증(UE Automation 직접) → [N] CI 그린 + PM 종합 + 머지. PM 자율([[feedback-autonomous-slices]], [[feedback-ci-before-merge]]).

**후속:** 등급별 차등, 즉시 회복형, 오프라인 일시정지, 자동 사용, 소비 제작.
