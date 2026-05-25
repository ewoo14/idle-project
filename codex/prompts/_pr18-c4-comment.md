## Codex TM 리뷰 + fix 산출 (단계 [4])

커밋 `3f87ef8` `codex(qa): add quest parity and objective coverage`

### Claude TM 권장 처리 + 독립 보강
- **서버↔클라 정의 parity 테스트**: 서버 Vitest(quests.ts 메타데이터) + UE Automation `DefinitionParity` — questId별 target/reward/objective/type/prerequisite 동일성 검증.
- **objective 진행 보강**: `QuestService.addProgressForObjective()` + UE `RecordGearEnhanced` 훅 검증 + 메인 체인/일일 진행 테스트.
- **독립 발견 fix**: claimed 퀘스트가 objective 진행에서 재저장/재진행되지 않도록 처리(중복/되감기 방지).
- QA 시나리오 Given/When/Then 갱신.

### 검증
- 서버 `npm test` **104 passed | 1 skipped**, tsc 0, lint clean.
- UE `Build.bat` Succeeded, Automation **29 tests** (DefinitionParity/QuestRewardAndHooks 포함) Success, EXIT 0.

→ 다음: Claude 검증(단계 [5]) — PM 직접 재검증.
