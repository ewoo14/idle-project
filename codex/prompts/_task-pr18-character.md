퀘스트 시스템 V1 의 클라이언트(C++) 진행 훅 + 수령을 구현하라. 기획서: docs/planning/slices/18-quest-system-v1.md. 서버 정의 참조: server/src/core/data/quests.ts (questId/type/objective/targetCount/reward/prerequisite). 기존 패턴: client/Source/IdleProject/GameCore/IdleGameInstance, CharacterSystem/IdleMonster(HandleDeath), CombatSystem.

구현 범위 (이번 character 호출):
1. 클라 퀘스트 상태/진행: UIdleGameInstance 또는 신규 GameCore/QuestService(컴포넌트/UObject) — 활성 퀘스트 보유(메인 체인 현재 단계 + 일일 3), questId→progress/completed/claimed. 서버 quests.ts 정의를 C++ 데이터로 미러(또는 최소 하드코딩, 서버와 동일 id/objective/target).
2. 진행 훅:
   - 몬스터 처치(AIdleMonster::HandleDeath 또는 GameInstance 의 기존 EXP 지급 경로) → objective=kill_monster 활성 퀘스트 progress +1.
   - 오프라인 수령(ClaimOfflineRewards) → claim_offline objective +1. 장비 강화 경로 → enhance objective +1.
3. 완료/수령: progress>=target 이면 완료, ClaimQuest(questId) → 보상(골드/EXP) 기존 AddGold/AddExp 반영 + claimed, 메인이면 다음 선행 해금. 일일 리셋(UTC 자정, 마지막 리셋일 비교)도 클라에서 lazy 처리.
4. (서버 graceful) NetworkClient 로 /v1/quests, progress, claim 호출 가능하면 동기화, 없으면 로컬. V1 로컬로 동작.
5. 자동화 테스트(Tests/): 진행 누적/완료/중복수령 방지/메인 체인 해금/일일 리셋(날짜 경계) 순수 로직. CombatTests 패턴.

제약: 한글 주석, "로직 C++". 퀘스트 로그 UI(위젯/HUD)는 designer 후속 — character 는 데이터/진행/수령 로직까지. 가능하면 Build.bat + Automation 검증. push 금지. 커밋 prefix: codex(character):.
