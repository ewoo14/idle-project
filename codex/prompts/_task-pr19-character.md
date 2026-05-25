챕터1 보스 + 환생 V1 의 캐릭터·전투 코어(C++)를 구현하라. 기획서: docs/planning/slices/19-boss-rebirth-v1.md. 기존: CharacterSystem/IdleMonster, IdleCharacter, StatFormulas, GameCore/IdleGameInstance, IdleProjectGameModeBase, CombatSystem.

구현 범위 (이번 character 호출):
1. 보스 "안개 군주":
   - AIdleMonster 에 bIsBoss(또는 보스 파라미터) + 강화 스탯(높은 HP/ATK) 구성. 또는 GameMode 에서 챕터1 관문 보스 spawn. V1 광역 패턴은 주기적 강타로 단순화 허용.
   - 보스 격파 시 UIdleGameInstance 에 bChapter1BossDefeated 플래그 set(세이브 반영).
2. 환생 V1 (UIdleGameInstance 또는 GameCore/RebirthService):
   - CanRebirth() = bChapter1BossDefeated && Level >= 100.
   - Rebirth(): rebirthCount++, RebirthBonusPoints += 5, Level = 1(리셋), Gold = floor(Gold * 보존율(예 0.1)), 장비 일부 보존(규칙 — V1 단순: 장착 장비 유지/인벤 일부), 능력치에 RebirthBonusPoints 영구 반영.
   - RefreshDerivedStats(또는 StatFormulas 합산 지점)에서 RebirthBonusPoints 를 모든 능력치에 가산(영구 보너스).
   - GetRebirthCount()/GetRebirthBonusPoints() 조회.
3. rebirthCount 가 오프라인 보너스(OfflineRewardFormula RebirthCount 인자)·표시에 반영되도록 연결 확인.
4. Automation 테스트(Tests/): CanRebirth 조건(보스 미격파/Lv 미달 → false), Rebirth 효과(rebirthCount/포인트/레벨 리셋/골드 보존율/스탯 반영), 다회 환생 누적. CombatTests 패턴.

제약: 한글 주석, "로직 C++". 환생 UI(위젯)·서버 persist 는 후속 보조 호출 범위 — character 는 게임플레이 로직까지. 보존율/포인트 수치는 기획서 1차값 사용(밸런스 후속). 가능하면 Build.bat + Automation 검증. push 금지. 커밋 prefix: codex(character):.
