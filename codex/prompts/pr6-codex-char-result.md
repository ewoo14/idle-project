## Codex PR #6 캐릭터 메인 결과

### 추가/수정 파일 + 커밋 목록
푸시 완료: `origin/plan/06-auto-battle-v1`

- `6f0028e` `codex(character): CombatComponent + CombatFormulas 추가`
- `7f52411` `codex(character): BattleAIComponent 자동 전투 AI`
- `2e0097b` `codex(character): UIdleGameInstance Gold/Exp/Level`
- `76bea5d` `codex(character): AGoldDrop 자동 흡수`
- `67ebf89` `codex(character): AIdleMonster 슬라임 placeholder`
- `cf1b791` `codex(character): UApiClient guest register 첫 호출`
- `48e3b23` `codex(character): IdleHUD 베이스 추가`
- `724b741` `codex(character): GameMode 몬스터 spawn + respawn`

### 주요 구현
- `CombatSystem`: `UCombatComponent`, `FCombatFormulas`, `UBattleAIComponent`
- `CharacterSystem`: `AIdleCharacter` 전투/AI 통합, `AIdleMonster` 신규
- `ItemSystem`: `AGoldDrop` 자동 흡수 및 `UIdleGameInstance::AddGold`
- `GameCore`: Gold/Exp/Level 상태와 델리게이트, Init 시 guest register 호출
- `NetworkClient`: `/v1/auth/register` 비동기 guest 등록, 서버 미기동 graceful log
- `GameMode`: 초기 몬스터 5마리 spawn, 사망 위치 5초 respawn
- `UI`: `AIdleHUD` 베이스 및 GI 델리게이트 구독
- `DefaultEngine.ini`: HUDClass, DefaultPawnClass, ServerBaseUrl 설정 hunk 반영

### 자기 검증
- RED 확인: `CombatTests.cpp` 추가 후 `CombatSystem/CombatFormulas.h` 없음으로 빌드 실패 확인
- Build: `Build.bat IdleProjectEditor Win64 Development ...` 결과 `Succeeded`
- Automation: `Automation RunTests IdleProject` 결과 5개 테스트 모두 `Success`, `TEST COMPLETE. EXIT CODE: 0`
- UENUM: `EBattleState`에 `None = 0 UMETA(Hidden)` 적용
- TestEqual: Combat float 기대값 `.0f` 적용
- include: 모듈 루트 기준 경로 사용, `../`/`Source` include 없음
- BP/.uasset 생성 없음

### 알려진 한계
- HUD는 요청 범위대로 Slate 구현 전 베이스만 있습니다.
- `AutoBattleSimTests.cpp`는 선택 범위라 추가하지 않았습니다.
- 워킹트리에 기존 미커밋 변경이 남아 있습니다: `client/Config/DefaultEngine.ini`의 AndroidFileServer 설정, `client/Config/DefaultInput.ini`, `codex/prompts/pr6-character-main.md`, `codex_char6_output.tmp`. PR #6 커밋에는 포함하지 않았습니다.
- Automation 로그 초반에 UE 자체 `LogAutomationTest: Error: Condition failed` 4줄이 남지만, 테스트 세션은 5개 전부 Success 및 Exit Code 0으로 종료됩니다.
