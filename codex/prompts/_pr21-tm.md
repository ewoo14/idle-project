## Claude TM 종합 리뷰 (단계 [3])

PM 직접 전체 검증.

### ✅ 칭찬
- **검증 GREEN**: 서버 vitest **113 passed**, UE Build Succeeded, UE Automation **38/38 Success**(신규: 마법사/궁수 스킬 로드, ClassId 분기 로딩, 직업 선택 HUD).
- **기존 인프라 재사용**: StatFormulas 3직업 성장곡선 활용, SkillComponent LoadDefault* 패턴, SkillDB 미러 확장. 직업 선택 → ClassId 변경 → 스킬/스탯 재로딩.
- **scope 적정**: 마법/크리 계산 과설계 없이 V1 계수 근사.

### 🟡 권장 (→ Codex [4] 흡수)
1. **서버↔클라 스킬 parity 확장**: 기존 DefinitionParity(전사)를 마법사/궁수까지 — 3직업 21스킬 id/type/effectType/cooldown/계수 동일성 검증.
2. **직업 스토리 분기**(story): 스토리 바이블 §3 또는 직업 섹션에 마법사/궁수 배경 1문단 + 텍스트 키, 챕터1 진입 직업별 한 줄.
3. **밸런스 문서**(balance): 마법사(고DPS·종이) / 궁수(크리) 직업 밸런스 1차값 표 + 시뮬레이터 후속 명시.

### 🔍 검증 잔여
- 사용자 PIE(차후): 직업 선택 → 직업별 스킬/스탯 체감.

### 판정
**블로커 0.** → Codex [4]에서 parity 확장 + 스토리 분기 + 밸런스 문서 후 Claude 검증 → **CI 그린 확정** 머지.
