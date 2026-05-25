## Claude TM 종합 리뷰 (단계 [3])

PM 직접 전체 검증.

### ✅ 칭찬
- **검증 GREEN**: 서버 vitest **122 passed**, UE Build Succeeded, UE Automation **44/44 Success**(신규: PetService EquipBonus/DefinitionParity, SeasonService TierClaim, PetSeasonHooks, PetSeasonViewModels).
- **펫**: 강아지 골드+20%/새 드롭+15% 장착 보너스 → IdleMonster 골드/드롭 반영. 서버↔클라 미러(DefinitionParity).
- **시즌 패스**: 무료 10티어, 시즌 토큰 누적/티어/수령(중복 방지), ClaimQuest→토큰+10 적립(퀘스트 연동). 펫/시즌 HUD.

### 🟡 권장 (→ Codex [4] 흡수)
1. **밸런스 문서**: 펫 %/시즌 티어 요구 토큰·보상 1차 표 + 시뮬레이터 후속 명시.
2. **QA 시나리오**: 펫 장착→보너스 / 시즌 토큰→티어→수령 Given/When/Then.
3. **parity/경계 보강**: 펫 보너스 서버↔클라 동일 % spot-check, 시즌 최종 티어/토큰 초과 경계.

### 🔍 검증 잔여
- 사용자 PIE(차후): 펫 장착 보너스 체감 + 시즌 패스 진행/수령.

### 판정
**블로커 0.** → Codex [4](문서/시나리오/parity) 후 Claude 검증 → **CI 그린 확정** 머지. **M5 완성 슬라이스**.
