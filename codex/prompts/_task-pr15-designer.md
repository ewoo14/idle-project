스킬 트리 V1 의 HUD 쿨다운/궁극기 게이지 표시를 구현하라. 기획서: docs/planning/slices/15-skill-tree-v1.md §2.6. 이미 머지된 character 산출(USkillComponent)을 사용한다.

대상: client/Source/IdleProject/UI/IdleHUD (기존 HUD 확장). C++ HUD(DrawHUD) 방식 — 기존 HP/EXP/상태 그리던 패턴을 따른다.

USkillComponent 공개 API (client/Source/IdleProject/CombatSystem/SkillComponent.h):
- public TArray<FSkillDefinition> Skills; (각 FSkillDefinition: SkillId(FName), DisplayName(FText/FString), Type(ESkillType{Active,Passive,Ultimate}), Cooldown 등)
- float GetCooldownRatio(FName SkillId, float Now) const; // 0(준비)~1(방금 발동)
- float GetCooldownRemaining(FName SkillId, float Now) const;
- float GetCurrentGauge() const; // 0~100
- bool IsUltimateReady() const;

구현:
1. 플레이어(AIdleCharacter)의 USkillComponent 를 찾는다(FindComponentByClass<USkillComponent>). World->GetTimeSeconds() 를 Now 로 사용.
2. 화면 하단에 액티브 스킬(Type==Active) 각각의 쿨다운을 표시: 이름/약자 + 진행 바 또는 남은초. GetCooldownRatio 로 채움 비율 표시(준비되면 강조).
3. 궁극기 게이지 바: GetCurrentGauge()/100 비율로 채우고, IsUltimateReady() 면 "READY" 강조.
4. V1 은 아이콘 에셋 없이 텍스트 + 단순 사각 바(DrawRect/DrawText)로 충분. 색상은 design-system 토큰(docs/planning/ui-tokens.json) 참고하되 하드코딩 허용.

추가:
- docs/planning/04-art-direction.md 에 스킬 아이콘/이펙트 방향(후속 아트) 1~2문단 추가.
- 가능하면 Build.bat 로 컴파일 확인(에디터 닫힘).

제약: 한글 주석/문서. 전투·스킬 로직(SkillComponent/BattleAI) 은 수정하지 말 것(조회만). 밸런스 수치/서버/테스트는 범위 외. 커밋 prefix: codex(designer):.
