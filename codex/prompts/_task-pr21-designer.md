마법사+궁수 직업 V1 의 직업 선택 UI 를 구현하라. 기획서: docs/planning/slices/21-mage-archer-classes-v1.md §2.2. 이미 머지된 character 산출 사용(USkillComponent LoadSkillsForClass(EClassId), AIdleCharacter DefaultClassId, EClassId Warrior/Mage/Archer).

구현:
1. 직업 선택 패널(IdleHUD DrawHUD 확장 또는 위젯): 전사/마법사/궁수 3개 선택 버튼 + 각 직업 한 줄 설명(전사 STR·CON 근접, 마법사 INT 폭발마법, 궁수 DEX 크리). 현재 선택 직업 강조.
2. 선택 → AIdleCharacter 의 ClassId 변경 + LoadSkillsForClass 재로딩 + RefreshDerivedStats(스탯 반영). (직업 변경 시 스킬/스탯이 즉시 갱신되어야 함.)
3. V1 진입점: 시작 시 직업 선택 또는 단축키로 패널 토글(기존 HUD 단축키 컨벤션 참고). 한글 라벨("직업 선택", "전사/마법사/궁수").
4. docs/planning/04-art-direction.md 에 직업 선택 패널 V1 1문단.

제약: 한글 주석/라벨. 스킬/스탯 로직(SkillComponent/StatFormulas/IdleCharacter 코어)은 조회·LoadSkillsForClass/RefreshDerivedStats 호출만(로직 변경 금지). 가능하면 Build.bat + Automation(UI 모델) 검증. push 금지. 커밋 prefix: codex(designer):.
