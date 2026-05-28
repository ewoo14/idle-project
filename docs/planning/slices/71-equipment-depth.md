# PR #71 기획서 — 장비 심화 (강화 리스크 + 잠재 레이어)

> **PM 자율 진행**(사용자 "PM 알아서 계속"). 브레인스토밍→스펙→계획으로 확정한 통합 장비 심화 슬라이스. 강화에 단계 하락 리스크·보호서·천장을 도입(영구 파괴 없음, 스타포스 스타일)하고, 드롭 어픽스(#40/#58)와 분리된 **별도 잠재 레이어**를 큐브로 리롤. 무한 성장([[project-infinite-growth]]) + 콘텐츠 풍부화([[project-content-richness]]). client + server 멀티시스템(7파트).
>
> 상세 스펙: [`docs/superpowers/specs/2026-05-28-equipment-depth-design.md`](../../superpowers/specs/2026-05-28-equipment-depth-design.md)
> 구현 계획(TDD): [`docs/superpowers/plans/2026-05-28-equipment-depth.md`](../../superpowers/plans/2026-05-28-equipment-depth.md)

## 1. 목표 / DoD

방치형 12+ 톤을 유지하며 **이미 보유한 장비와 계속 상호작용할 이유**를 만든다 — 강화 수직 긴장 + 잠재 수평 추구.

### DoD 검증
1. **강화 리스크**: 안전 구간 +0~+9(실패 단계 유지), 위험 구간 +10~+50(실패 시 -1단계). 보호서 사용 시 위험 실패에도 단계 유지(보호서 소모). 천장: 위험 구간 같은 단계 연속 실패 12회 시 다음 시도 강제 성공. 비용·성공률 곡선·`1+EnhanceLevel*0.1` 보상은 불변. 클라 `FEnhanceFormula::ResolveAttempt` ↔ 서버 `resolveEnhanceAttempt` parity.
2. **잠재 레이어**: Rare+ 아이템만 잠재 보유. 잠재 등급(Rare~Legendary) 상한은 아이템 등급 연동(Rare→Epic, Epic→Unique, Unique→Legendary, Legendary+→Legendary). 줄 수=등급(1/2/3/3). % 옵션 풀. 드롭 어픽스 불간섭. 클라 `PotentialFormula` ↔ 서버 `potential.ts` parity.
3. **큐브 리롤**: 재설정 큐브(등급 유지·줄 재롤), 등급 큐브(줄 재롤 + 낮은 확률 등급 상승, 상한 내). `TryRerollPotential`. 미리보기→적용/유지 UX.
4. **신규 자원**: 보호서/재설정 큐브/등급 큐브. 드롭·던전/보스·골드 상점 교환 수급(가챠 없음). 골드 상점 PR #38 곡선 정합.
5. **PowerScore/자동장착**: 잠재가 파생 스탯(% 승수, 유니크 trait 경로)에 반영 + PowerScore 가중 합산(클라/서버 동일). 아이템 잠금(🔒) 시 자동장착 교체 제외.
6. **저장**: `FItemInstance`에 PotentialGrade/PotentialLine1~3/EnhanceFailStreak/bLocked 추가, `UIdleSaveGame`에 자원 3종. SaveVersion **11→12** 마이그레이션(구 세이브 기본값). 클라우드 페이로드 반영. 라운드트립 회귀안전.
7. **테스트**: 클라 Automation(리스크 경계·보호·천장·잠재 게이팅/롤·큐브·PowerScore·잠금·v11→v12) + 서버 vitest(enhance/potential/drop/equipment parity) + balance-sim(신규 실패 모델 재실행, 첫 환생 5~10h 밴드 유지). UE Build/Automation + server-ci **GREEN**.

## 2. 범위 (In Scope)
### 2.1 강화 리스크 (character + backend)
구간 판정·하락·천장·보호 공식(클라/서버 미러), `TryEnhanceEquipped` 확장(보호 토글·보호서 차감), `InventoryComponent` 결과 적용.
### 2.2 잠재 레이어 (character + backend)
`PotentialFormula`/`potential.ts`(게이팅·줄 롤·큐브), 드롭 부여, `TryRerollPotential`, DeriveStats % 승수, PowerScore 가중.
### 2.3 경제 (balance)
보호서/큐브 수급·가격(1차 시안), 등급 큐브 상승 확률(8% 시안), 잠재 롤 폭, PowerScore 가중치. balance-sim 재실행.
### 2.4 UI (designer)
강화 패널(구간/보호/천장), 잠재 패널(미리보기/적용/유지), 잠금 토글, 잠재 등급 색상, 로컬라이즈 ko/en.
### 2.5 저장/마이그레이션 (character) — DoD 6.
### 2.6 테스트 — DoD 7.

## 3. 범위 외 (후속)
- 서버 권위 지출 검증(전 경제 시스템 일괄 후속 슬라이스).
- 드롭 어픽스 풀/확률/수치 변경.
- 큐브 2종→1종 통합, 미리보기→즉시교체 전환(구현 중 PM 확정 여지).
- 잠재 옵션 추가 풀·잠재 줄 고정(lock) 화폐.

## 4. 작업 분배 — Codex 호출
| 파트 | 작업 |
| --- | --- |
| character (메인) | FItemInstance 필드·enum·struct, FEnhanceFormula 리스크/천장/보호, PotentialFormula, InventoryComponent(결과 적용/잠재/잠금/자동장착), IdleGameInstance(TryEnhanceEquipped 보호·TryRerollPotential·자원·세이브 v11→v12), DeriveStats 잠재, PowerScore, 클라 Automation |
| backend | enhance.ts(resolveEnhanceAttempt), potential.ts, drop.ts(PowerScore), equipment.ts(잠재 파생), index.ts, vitest parity |
| balance | 보호서/큐브 가격·수급, 잠재 롤 폭, 등급 큐브 확률, PowerScore 가중치, balance-sim 재실행 결과 |
| designer | 강화/잠재 패널 + 잠금 토글 + 잠재 등급 색상 + 로컬라이즈 ko/en |
| qa | 시나리오(보호/천장/큐브/잠금/마이그레이션) + 회귀 체크리스트 |
| (story/quest) | 해당 없음(기존 강화 퀘스트 RecordGearEnhanced 유지) |

## 5. 워크플로우 v3
[1] 기획+PR → [2] Codex character 메인(+backend/balance/designer/qa) → PM 산출 게시 → [3] Claude TM 종합+fix → [4] Codex TM 종합+fix+PM 산출 게시 → [5] 검증(UE Automation 직접 구동, #68 교훈) → [N] **CI 그린 확정** + PM 종합 소견 + 머지. PM 자율.

## 6. 리스크
| 리스크 | 완화 |
| --- | --- |
| 강화 하락 도입으로 후반 페이싱 급변 | 비용/성공률 곡선 불변 + balance-sim 재실행으로 +50 기대비용·첫 환생 밴드 확인 |
| 천장 리셋 규칙 모호(보호 실패 누적 vs 하락 리셋) | 스펙 §2.3 명세대로 구현(성공/하락 시 리셋, 보호 실패 누적) + 경계 Automation/vitest |
| 잠재 % 승수 ↔ PowerScore 정합(클라/서버) | 동일 가중치 상수 + parity 테스트(드롭 PowerScore 앵커) |
| 잠재 부여가 드롭 어픽스 밸런스 침범 | 별도 RNG·별도 필드, 어픽스 경로 불변(회귀 테스트) |
| 세이브 v11→v12 회귀 | 신규 필드 기본값 마이그레이션 + 라운드트립 + 클라우드 페이로드 테스트 |
| 자동장착이 투자 잠재템 교체 | PowerScore 잠재 반영 + bLocked 자동장착 제외 Automation |
| float fround 클라/서버 불일치 | 잠재 롤·가중치 fround 매칭(어픽스 #40 방식) |

## 7. 후속
- 전 경제 시스템 서버 권위 지출 검증.
- 잠재 줄 고정(lock) 화폐, 잠재 옵션 풀 확장.
- 큐브/보호서 텔레메트리 기반 수급·가격 재보정.
