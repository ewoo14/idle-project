# PR #81 기획서 — 소비 아이템 등급별 차등 (V1.5)

> **PM 자율 진행 (Claude 에이전트 구현팀, Codex 6/1 복구까지)**. #73 소비 아이템 후속 — 6종 소비에 **등급(소/중/대)** 차원을 추가해 효과 % 차등. 콘텐츠 풍부화([[project-content-richness]]). client + server 5-team.

## 1. 목표 / DoD
#73 소비는 타입당 단일 효과. V1.5는 등급별 차등(소/중/대)으로 깊이 추가.

### DoD
1. **등급**: `EConsumableGrade { Lesser=0, Standard=1, Greater=2 }`(소/중/대). `GetBuffPercent(type, grade)`/`GetBuffDurationSec(type, grade)` — Standard=#73 기존값, Lesser=×0.5, Greater=×2.0(% 차등; 지속은 타입별 고정 유지). 클라 `FConsumableFormula` ↔ 서버 `consumable.ts` parity.
2. **보유/사용**: 카운트는 (type, grade) 키. 사용 시 해당 등급 % 로 버프 발동. 버프 상태는 타입별 `{종료시각, 활성 등급}`(같은 타입 재사용 시 최신 사용 등급·지속으로 갱신). UBuffService 확장.
3. **수급**: 골드 상점이 3등급 판매(가격 차등, #38 곡선), 드롭은 등급 가중(Lesser 위주). 기존 수급 경로 확장.
4. **저장**: `FConsumableSaveEntry`에 `uint8 Grade` 추가. (type,grade) 카운트 + 타입별 활성 버프(종료시각+활성등급). SaveVersion **15→16**(v15 엔트리 = Standard 등급 마이그레이션). 클라우드 payload.
5. **UI**: 소비 패널에 등급 표시/선택(보유 등급별), 활성 버프에 등급 표기. ko/en CsvIntegrity.
6. **테스트**: 클라 Automation(등급별 %·사용·버프 등급·v15→v16·회귀) + 서버 vitest + parity. CI GREEN + **표준 jumbo 빌드 PM 검증**.

## 2. 작업 분배 (Claude 서브에이전트, claude(<role>):)
| 파트 | 작업 |
| --- | --- |
| character (메인) | EConsumableGrade, FConsumableFormula(type,grade) + 서버 consumable.ts 미러, UBuffService 등급 키 카운트/활성등급, GameInstance 사용/수급(상점 3등급/드롭 가중), 세이브 15→16, 클라 Automation |
| backend | consumable.ts (type,grade) 미러 + vitest + save.schema payload(grade 포함) |
| designer | 소비 패널 등급 표시/선택 + 활성 버프 등급 + ko/en |
| balance | 등급 % 차등(0.5/1/2)·상점 가격·드롭 가중 + 무한 성장 비침범 |
| qa | 등급별 효과/사용/버프 등급/스택/v15→v16/회귀 + parity |

## 3. 범위 외
즉시 회복형 소비, 등급별 지속시간 차등(V1.5는 % 만), 소비 제작/연금술.

## 4. 워크플로우 v3
Claude 서브에이전트 구현 → PM 리뷰/통합. [5] **표준 jumbo(unity) 빌드 PM 직접 검증**. 머지 전 CI 그린.

## 5. 리스크
| 리스크 | 완화 |
| --- | --- |
| 세이브 15→16 (type,grade) 마이그레이션 | v15 엔트리=Standard 매핑 + 라운드트립/마이그레이션 Automation |
| 버프 활성 등급 복원 정합 | 버프 상태에 활성 등급 저장 + 만료/재사용 경계 테스트 |
| 등급 % 무한 성장 침범 | 시간제·소모형 유지(영구 성장 직교), balance 문서 |
| 서버↔클라 drift | consumable.ts (type,grade) Math.fround 미러 + parity |
| jumbo ODR | 신규 익명 헬퍼 동명 grep + PM 표준 jumbo 빌드 |
