펫 2종 + 시즌 패스 베타 V1 의 클라이언트(C++) 보너스/토큰 로직을 구현하라. 기획서: docs/planning/slices/22-pets-season-pass-v1.md. 서버 데이터 참조(동일 상수 미러): server/src/core/data/pets.ts(강아지 gold +20%, 새 drop +15%), server/src/core/data/season.ts(무료 10티어 requiredTokens/reward). 기존: GameCore/IdleGameInstance, CharacterSystem/IdleMonster(GoldDrop/HandleDeath), GameCore/QuestService(시즌 토큰 적립 훅).

구현 범위 (이번 character 호출):
1. 펫 시스템(GameCore/PetService 또는 GameInstance): 펫 2종 정의 C++ 미러(서버 동일), 장착(1마리 V1), GetEquippedPetGoldBonusPercent()/GetDropBonusPercent().
   - 골드 보너스: IdleMonster GoldDrop 또는 GameInstance 골드 적립 시 (1 + goldBonus%) 반영. 드롭 보너스: 장비 드롭 확률에 (1 + dropBonus%) 반영.
2. 시즌 패스(GameCore/SeasonService 또는 GameInstance): 시즌 토큰 잔량/누적, 무료 티어 정의 C++ 미러, 티어 진행(누적 토큰→현재 티어), ClaimSeasonReward(tier)→보상(골드/EXP) 반영+중복 방지.
   - 시즌 토큰 적립 훅: QuestService 퀘스트 완료/수령(ClaimQuest) 시 시즌 토큰 +N(예 +10). 연결.
3. (서버 graceful) NetworkClient /v1/pets·/v1/season 호출 가능하면 동기화, 없으면 로컬. V1 로컬 동작.
4. Automation(Tests/): 펫 골드/드롭 보너스 계산, 시즌 토큰 누적→티어 도달→수령(중복 방지), 퀘스트 완료→토큰 적립 훅. 서버 데이터와 parity 스폿체크. CombatTests 패턴.

제약: 한글 주석, "로직 C++". UI(펫/시즌 위젯)는 designer 후속. 수치는 서버 미러(변경 금지). 가능하면 Build.bat + Automation 검증. push 금지. 커밋 prefix: codex(character):.
