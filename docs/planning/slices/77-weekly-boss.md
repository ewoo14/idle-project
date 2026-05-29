# PR #77 기획서 — 주간 보스 (Weekly Boss)

> **PM 자율 진행**. 신규 엔드게임 반복 콘텐츠(주간 cadence). 매주 리셋되는 보스에게 CP 기반 누적 데미지(주 7회)를 쌓아 무한 마일스톤 보상을 받는다. 일일 던전(#68/#75)·영구 탑(#50/#51)과 다른 주간 단위 목표. 무한 성장([[project-infinite-growth]]). client + server 5-team.
>
> 스펙: [`docs/superpowers/specs/2026-05-29-weekly-boss-design.md`](../../superpowers/specs/2026-05-29-weekly-boss-design.md)
> 계획(TDD): [`docs/superpowers/plans/2026-05-29-weekly-boss.md`](../../superpowers/plans/2026-05-29-weekly-boss.md)

## 1. 목표 / DoD
주간 단위 반복 목표(리텐션 벡터) 신설. CP가 도전 데미지로 반영되고 무한 마일스톤으로 보상.

### DoD 검증
1. **도전**: `TryChallengeWeeklyBoss()` — 주당 7회 제한, 1회 데미지 = CP. 새 ISO week 진입 시 누적/도전/수령 리셋(`UQuestService::GetCurrentUtcWeekString`).
2. **무한 마일스톤**: `MilestoneThreshold(n)=floor(1000×1.5^(n-1))`, `GetReachedMilestones(누적)`. `ClaimWeeklyBossMilestone(n)` 도달&미수령 시 보상(골드/에센스). 클라 `FWeeklyBossFormula` ↔ 서버 `weeklyBoss.ts` parity(Math.fround).
3. **저장**: SaveVersion **14→15**(WeekId/Damage/ChallengesUsed/ClaimedMilestones), v14 누락=0 마이그레이션, 클라우드 payload.
4. **UI**: 주간 보스 패널(누적 vs 다음 마일스톤 게이지, 도전 잔여/버튼, 마일스톤 목록/수령, 주 리셋) + ko/en CsvIntegrity.
5. **테스트**: 클라 Automation(도전/누적/한도/마일스톤 수령 중복차단/주 리셋/v14→v15/회귀) + 서버 vitest + parity. UE Build/Automation + server-ci GREEN. **표준 jumbo(unity) 빌드 PM 직접 검증**(#76 교훈).

## 2. 범위 (In Scope)
2.1 공식(character+backend) — weeklyBoss 미러.
2.2 서비스/진입(character) — UWeeklyBossService + GameInstance 도전/수령 + 세이브 v15.
2.3 UI(designer) — 주간 보스 패널 + ko/en.
2.4 밸런스(balance) — 도전/데미지/마일스톤/보상 곡선 문서.
2.5 테스트(qa) — DoD 5.

## 3. 범위 외 (후속)
- 주간 보스 데미지 리더보드(#76 연동), 보스 페이즈/속성 연출(#35), 다중 주간 보스 로테이션, 길드 협력 보스.

## 4. 작업 분배 — Codex 호출
| 파트 | 작업 |
| --- | --- |
| character (메인) | WeeklyBossTypes/FWeeklyBossFormula, UWeeklyBossService, GameInstance(TryChallenge/ClaimMilestone), 세이브 v14→v15, 클라 Automation |
| backend | weeklyBoss.ts 미러 + vitest parity + save.schema payload |
| designer | 주간 보스 패널 + ko/en + CsvIntegrity |
| balance | 도전 제한·데미지·마일스톤·보상 곡선 + 무한 비폭주 + 주간 페이싱 |
| qa | 도전/누적/마일스톤/주 리셋/v14→v15/회귀 + parity |

## 5. 워크플로우 v3
[1] 기획+PR → [2] Codex 5-team → PM 게시 → [3] Claude TM 종합+fix → [4] Codex(결함 시) → [5] 검증(**표준 jumbo(unity) 빌드 PM 직접**, #76 교훈) → [N] CI 그린 + PM 종합 + 머지. PM 자율([[feedback-autonomous-slices]], [[feedback-ci-before-merge]]).

## 6. 리스크
| 리스크 | 완화 |
| --- | --- |
| 마일스톤 보상 폭주 | 주 7회 제한 + 기하 임계 + balance |
| 주 리셋 경계 | ISO week 문자열 비교(#56 패턴) + 경계 Automation |
| jumbo(unity) ODR(#76) | 신규 익명 헬퍼 동명 grep + 표준 jumbo 빌드 PM 검증 |
| 세이브 14→15 회귀 | 누락=0 + 라운드트립 + 클라우드 payload |
| 서버↔클라 drift | weeklyBoss.ts Math.fround 미러 + parity |
| CP-보상 루프 중복감 | 주간 cadence·누적·리셋으로 일일/영구와 차별 |

## 7. 후속
주간 보스 데미지 리더보드(#76), 보스 페이즈 연출, 다중 보스 로테이션, 길드 협력.
