## 개요 ([1] PM 기획)
M5 — 마법사 + 궁수 직업 추가. 스탯 성장곡선은 기존(StatFormulas Warrior/Mage/Archer). 이번엔 **직업별 스킬 7종 + ClassId 기반 스킬 로딩 + 직업 스토리 분기 + SkillDB 미러 + 직업 선택**.

기획서: `docs/planning/slices/21-mage-archer-classes-v1.md`.

## 설계
- character(메인): LoadDefaultMageSkills/ArcherSkills(active4/passive2/ult1) + AIdleCharacter DefaultClassId 분기 로딩 + Automation.
- balance(수치) / story(직업 분기) / backend(SkillDB 미러) / designer(직업 선택 UI).

## 워크플로우 v3
[1]→[2] Codex character 메인+balance/story/backend/designer/qa →[3] Claude TM →[4] Codex TM+fix →[5] 검증 →[N] **CI 그린 확정** 후 머지. 사용자 PIE 차후 일괄.

🤖 Generated with [Claude Code](https://claude.com/claude-code)
