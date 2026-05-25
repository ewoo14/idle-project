스킬 트리 V1 의 캐릭터·전투 코어를 구현하라. 기획서: docs/planning/slices/15-skill-tree-v1.md (반드시 먼저 읽고 그대로 따른다). 기존 코드 패턴 준수: client/Source/IdleProject/CombatSystem/ (CombatComponent, BattleAIComponent, CombatFormulas), CharacterSystem/IdleCharacter, StatFormulas. "로직은 C++".

구현 범위 (이번 character 호출):
1. CombatSystem 에 스킬 코어 추가:
   - 열거형 ESkillType{Active,Passive,Ultimate}, ESkillEffectType{DamageSingle,DamageAoe,SelfBuff,DashDamage}
   - FSkillDefinition (SkillId(FName), DisplayName, Type, EffectType, Cooldown, DamageCoeff, BuffMagnitude, BuffDuration, GaugeGainOnHit, GaugeGainOnTakeDamage)
   - USkillComponent : UActorComponent — 보유 스킬 목록 + 런타임상태(스킬별 LastCastTime, 궁극기 CurrentGauge 0~100).
     * TickSkills(float Now, AActor* Target, TArray<AActor*> AoeTargets) — 준비된 액티브 자동발동.
     * IsReady(FName,Now)/GetCooldownRemaining/GetCooldownRatio — UI 조회.
     * AddGauge(float), IsUltimateReady(), 발동 시 0 리셋.
     * ApplyPassivesToStats(FDerivedStats&) — 패시브 합산(ATK+15%, MaxHP+20%).
     * 전사 기본 7종 정의를 코드 테이블로 보유(StatFormulas 패턴). 수치는 기획서 2.4 표 그대로.
   - 데미지는 FCombatFormulas::ComputeDamage 재사용.
2. BattleAIComponent::UpdateBattle 에서 USkillComponent::TickSkills 호출(사거리 내 타깃 있을 때). 공격/피격 시 궁극기 게이지 누적 훅.
3. AIdleCharacter 에 USkillComponent 추가 + 전사 7종 주입, RefreshDerivedStats 에서 ApplyPassivesToStats 반영.
4. 자동화 테스트(client/Source/IdleProject/Tests/): 쿨다운 IsReady/GetCooldownRatio, 궁극기 게이지 누적/임계/리셋, 패시브 합산 — 순수 로직 단위테스트(CombatTests.cpp 패턴, IMPLEMENT_SIMPLE_AUTOMATION_TEST). FVector/float 오버로드 주의(double 통일).

제약: 한글 주석/도메인 메서드 컨벤션 준수. UI(HUD)/밸런스 데이터문서/서버 SkillDB/아트는 이번 호출 범위 외(후속 보조 호출). 가능하면 Build.bat 로 컴파일 확인(에디터 닫힘 상태). 커밋 prefix: codex(character):.
