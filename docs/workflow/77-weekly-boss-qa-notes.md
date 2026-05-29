# PR #77 주간 보스 — QA 노트 (Given/When/Then)

> PM/Claude 직접 작성(Codex 사용량 한도 소진 대체).

## 시나리오

### S1. 주간 도전 누적 / 한도
- Given 새 주(2026-W22) 시작, When CP 1000으로 7회 도전, Then 누적 데미지 7000·잔여 0·도달 마일스톤 5.
- Given 잔여 0, When 8회째 도전, Then 실패·데미지 불변. (커버: `IdleProject.WeeklyBoss.ServiceChallengeClaimReset`)

### S2. 마일스톤 수령 / 중복 차단
- Given 도달 5·수령 0, When `ClaimMilestone(3)`, Then 성공·최고 수령 3.
- When 동일/하위 재수령, Then 실패. When 미도달(6) 수령, Then 실패. (커버: 동 테스트)

### S3. 마일스톤 누적 지급 (보상 유실 방지, fix `0bb476e`)
- Given 수령 0·도달 ≥3, When `UIdleGameInstance::ClaimWeeklyBossMilestone(3)`, Then 골드 += goldReward(1)+goldReward(2)+goldReward(3) = 5000+7500+11250 = **23750**, 에센스 += 3+6+9 = **18** (단일 3 보상만 주던 회귀 수정).
- **검증**: 표준 jumbo(unity) 빌드 `Result: Succeeded`(PM 직접) + GameInstance 루프 `for(N=PreviousClaimed+1; N<=Milestone; ++N)` 코드 리뷰 + 서비스 `ClaimMilestone` 단위 테스트(수령=3 확정). (월드 백드 GameInstance Automation 은 후속 — Codex 한도 복구 후 보강 권장.)

### S4. 주 리셋
- Given 2026-W22 상태(데미지/도전/수령), When `EnsureWeek(2026-W23)`, Then 데미지 0·잔여 7·수령 0. (커버: 동 테스트)

### S5. 세이브 v14→v15
- Given v14 세이브(주간 보스 필드 없음), When 로드, Then 주간 보스 상태 0/빈. Given v15 라운드트립, Then WeekId/Damage/ChallengesUsed/ClaimedMilestones 보존. (커버: SaveSystem Automation v15/v14)

### S6. 서버↔클라 parity
- 임계/도달/보상/도전 데미지 앵커가 클라 `FWeeklyBossFormula` 기대값과 일치. (커버: `weeklyBoss.parity.test.ts`)

## 검증 명령
- 서버: `cd server; npm run lint && npm run test -- weeklyBoss && npm run balance:sim`
- 클라: 표준 jumbo(unity) 빌드(`-DisableUnity` 없이) + `Automation RunTests IdleProject.WeeklyBoss` + `IdleProject.GameCore.SaveSystem`

## 비고
- Codex 사용량 한도 소진(2026-06-01 복구)으로 balance/qa 파트는 PM/Claude 직접 작성. GameInstance 누적 수령 월드 백드 Automation 은 한도 복구 후 보강 대상.
