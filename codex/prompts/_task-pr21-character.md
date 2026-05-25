마법사+궁수 직업 V1 의 캐릭터·전투 코어(C++)를 구현하라. 기획서: docs/planning/slices/21-mage-archer-classes-v1.md. 기존: CombatSystem/SkillComponent(LoadDefaultWarriorSkills 패턴 + FSkillDefinition/ESkillType/ESkillEffectType), CharacterSystem/StatFormulas(EClassId Warrior/Mage/Archer + GetClassGrowth 이미 정의), IdleCharacter(DefaultClassId), CombatSystem/CombatFormulas.

구현 범위 (이번 character 호출):
1. USkillComponent 에 직업별 스킬셋 추가(LoadDefaultWarriorSkills 패턴 그대로):
   - LoadDefaultMageSkills(): 마법사 7종 — 액티브4(파이어볼=단일/체인라이트닝=AoE/마나실드=자버프/메테오=원거리강타), 패시브2(주문력+/최대MP+), 궁극기1(대마법). INT/마법 기반.
   - LoadDefaultArcherSkills(): 궁수 7종 — 액티브4(정조준=단일크리/멀티샷=AoE/집중=자버프/관통사격=대시형), 패시브2(크리율+/공속+), 궁극기1(화살비). DEX/크리 기반.
   - 수치(쿨다운/계수/버프/게이지)는 전사 스킬과 동급 밸런스로 1차 설정(기획서 컨셉 반영, balance 후속 보정).
   - LoadSkillsForClass(EClassId) 헬퍼로 분기(Warrior/Mage/Archer).
2. AIdleCharacter: BeginPlay 의 스킬 로드를 DefaultClassId 기반 분기(LoadSkillsForClass)로 변경(현재 Warrior 고정). 직업 변경 시 재로딩 가능하게.
3. 데미지: 기존 FCombatFormulas::ComputeDamage 사용. 마법/크리 분기는 V1 단순(마법사는 동일 공식에 계수, 궁수 크리는 단순 확률/배수 또는 계수로 근사) — 과설계 금지.
4. Automation(Tests/): 직업별 LoadDefault*Skills 7종 로드 검증, LoadSkillsForClass 분기(ClassId→스킬셋), 자동발동 스폿체크. CombatTests 패턴.

제약: 한글 주석, "로직 C++". 직업 선택 UI(designer)·서버 SkillDB(backend)·스토리(story)는 후속 보조 호출 범위. 스탯 성장은 기존 StatFormulas 사용(변경 금지). 가능하면 Build.bat + Automation 검증. push 금지. 커밋 prefix: codex(character):.
