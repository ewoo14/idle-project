# PR #73 기획서 — 소비 아이템 + 임시 버프 (Consumables & Timed Buffs)

> **PM 자율 진행**. brainstorming→스펙→writing-plans로 확정. GDD 전반에서 "소비 제외"로 빠져 있던 **미구현 핵심 카테고리** 도입 — 장비와 별개인 소비 아이템(6종)을 사용하면 일정 시간 능력치·획득량 **임시 버프**가 활성화. 콘텐츠 풍부화([[project-content-richness]]). client + server 5-team.
>
> 스펙: [`docs/superpowers/specs/2026-05-29-consumables-design.md`](../../superpowers/specs/2026-05-29-consumables-design.md)
> 계획(TDD): [`docs/superpowers/plans/2026-05-29-consumables.md`](../../superpowers/plans/2026-05-29-consumables.md)

## 1. 목표 / DoD

성장은 영구 시스템(장비/룬/펫/마스터리/환생)이 담당하므로, 소비 아이템은 **시간제 부스터**로 차별화(소모형·선택적). 미사용 시 기존 플레이 불변.

### DoD 검증
1. **6종 소비 아이템**: AttackTonic/GuardTonic/AllStatElixir(스탯) + FortuneScroll/GoldFeast/WisdomBooster(경제). 타입별 고정 %·지속(예 +30%/1800초). 클라 `FConsumableFormula` ↔ 서버 `consumable.ts` parity(Math.fround).
2. **시간제 버프**: 사용 시 종료 유닉스타임 설정, `Now<End` 활성. 스택(동일=연장, 다른=동시). 스탯 버프는 `RefreshDerivedStats` 기존 곱(transcend×tower×achievement×mastery)에 타입별 **별도 항** 합류, 경제 버프는 골드/EXP/드롭 경로 — **각 버프 단일 적용 지점**(이중 금지).
3. **수급/사용**: 드롭·던전(#68)·골드 상점(#38)·퀘스트 수급(카운터). 인벤토리 사용 → 카운터 -1 + 버프 발동 + 스탯 갱신.
4. **저장**: `UIdleSaveGame`에 타입별 보유 수량 + 활성 버프 종료시각. SaveVersion **13→14**(v13 누락=0 마이그레이션). 클라우드 payload 선택 필드. 환생/초월 리셋 비대상(보유 유지). 라운드트립 회귀안전.
5. **UI**: 소비 인벤토리(6종 보유·사용) + 활성 버프 바(아이콘+남은 시간). ko/en `CsvIntegrity`.
6. **테스트**: 클라 Automation(공식·서비스·사용→버프→스탯/경제↑→만료 원복·스택·v13→v14·리셋 생존·회귀) + 서버 vitest(consumable 공식/save payload) + parity. UE Build/Automation + server-ci **GREEN**.

## 2. 범위 (In Scope)
2.1 공식(character+backend) — `FConsumableFormula`/`consumable.ts` %·지속 미러.
2.2 서비스/사용(character) — `UBuffService`(보유·활성버프·getter), GameInstance 사용/수급 훅.
2.3 능력치/경제 합류(character) — RefreshDerivedStats 타입별 곱 + 골드/EXP/드롭.
2.4 저장 v13→v14(character+backend) — DoD 4.
2.5 UI(designer) — 인벤토리 + 버프 바 + ko/en.
2.6 밸런스(balance) — %·지속·수급·가격 + 보조성 문서.
2.7 테스트(qa) — DoD 6.

## 3. 범위 외 (후속)
- 등급별 차등 효과(소/중/대), 즉시 회복형(HP/MP), 오프라인 버프 일시정지, 자동 사용, 소비 제작(연금술), `EQuestObjective::UseConsumable`(선택 — 스코프 여유 시 quest 경량 확장).

## 4. 작업 분배 — Codex 호출
| 파트 | 작업 |
| --- | --- |
| character (메인) | ConsumableTypes/FConsumableFormula, UBuffService, GameInstance 사용·수급·RefreshDerivedStats 합류, 세이브 v13→v14, 클라 Automation |
| backend | consumable.ts 미러 + index.ts + save.schema payload + vitest |
| designer | 소비 인벤토리 + 버프 바 + ko/en + CsvIntegrity |
| balance | 버프 %·지속·수급·가격 + 보조성/페이싱 무변동 문서 |
| qa | E2E/스택/마이그레이션/리셋생존/parity/회귀 |

## 5. 워크플로우 v3
[1] 기획+PR → [2] Codex 5-team → PM 산출 게시 → [3] Claude TM 종합+fix → [4] Codex TM 종합+fix → [5] 검증(UE Automation 직접) → [N] **CI 그린 확정** + PM 종합 소견 + 머지. PM 자율([[feedback-autonomous-slices]], [[feedback-ci-before-merge]]).

## 6. 리스크
| 리스크 | 완화 |
| --- | --- |
| 버프 % 이중 적용(#72 교훈) | 각 % 단일 적용 지점 + getter 소비처 추적 + qa 원복 검증 |
| 시간제 버프 ↔ 세이브/오프라인 정합 | 종료 유닉스타임 절대값(실시간 소진) + 만료 경계 Automation |
| 영구 성장 잠식 | 보조 % + 지속 제한 + balance-sim median 무변동 확인 |
| 장비 인벤과 혼선 | FItemInstance와 분리된 카운터 모델, 장비 경로 불변 |
| 서버↔클라 drift | consumable.ts Math.fround 미러 + 경계 parity |

## 7. 후속
등급별 차등, 즉시 회복형, 오프라인 일시정지, 자동 사용, 소비 제작, UseConsumable 퀘스트.
