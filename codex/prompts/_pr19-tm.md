## Claude TM 종합 리뷰 (단계 [3])

PM 직접 전체 검증.

### ✅ 칭찬
- **검증 GREEN**: 서버 vitest **108 passed**, UE Build Succeeded, UE Automation **34/34 Success**(신규 환생/보스 로직 + RebirthPanelViewModel).
- **환생 스탯 보너스 C++↔TS 정합**: 양쪽 deriveStats 포인트당 HP+10/PhysAtk+2 동일.
- **연동**: rebirthCount → 오프라인 보너스(+5%/회) 기존 연동 유지. 환생 흐름(조건 보스격파+Lv100 → rebirthCount++/포인트+5/레벨 리셋/골드 10% 보존). 서버 persist + stale-update SQL 방지. 환생 HUD(조건/횟수/보너스/버튼) + 보스 표시.

### 🟡 권장 (→ Codex [4])
1. **환생 스탯 보너스 parity 테스트**: 서버 stats.test ↔ 클라 StatFormulas 테스트에서 동일 RebirthBonusPoints 입력 → HP/PhysAtk 동일 결과 spot-check(스킬/오프라인/퀘스트 parity 선례).
2. **다회 환생 누적 + 경계**: 포인트 누적, 골드 보존율(0.1) 경계, 레벨 리셋 후 deriveStats 정합 회귀.
3. **보스 격파 → 환생 자격 전이** 통합 시나리오(플래그 set → CanRebirth) 테스트.

### 🔍 검증 잔여
- 사용자 PIE(차후): 챕터1 보스 격파 → Lv100 → 환생 → 영구 강화 체감.

### 판정
**블로커 0.** parity·검증 견고 → Codex [4] (parity/경계 테스트) 후 Claude 검증 → 머지. **M4 마일스톤 완성 슬라이스**.
