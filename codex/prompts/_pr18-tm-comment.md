## Claude TM 종합 리뷰 (단계 [3])

PM 직접 전체 검증.

### ✅ 칭찬
- **검증 GREEN**: 서버 vitest **102 passed**, UE Build Succeeded, UE Automation **28/28 Success**(신규: QuestService 진행/수령/unlock·일일 리셋, GameInstance 퀘스트 훅, QuestLogViewModel).
- **퀘스트 ID parity**: 서버 `quests.ts` ↔ 클라 `QuestService` 8종(main_ch1_001~005 + daily_kill_monsters/claim_offline/enhance_gear) 일치.
- **설계**: UTC 일일 lazy 리셋, 메인 선행 체인 잠금/해금, 스토리 §5.1 맵 훅 기반 메인 퀘스트, 중복수령 방지 + 보상 트랜잭션. 진행 훅(킬/오프라인/강화) GameInstance 연동, 퀘스트 로그 HUD(Q 단축키).

### 🟡 권장 (→ Codex [4])
1. **서버↔클라 퀘스트 정의 값 parity 테스트**: questId 외 targetCount/rewardGold/rewardExp/objective/prerequisite 동일성 spot-check(오프라인 fix 선례). 값 어긋나면 진행/보상 불일치.
2. **objective 서버 권위 검증**: V1 은 클라 보고 수용 → 치트 여지. 후속 강화(기획 명시).
3. 진행 훅 objective 커버리지(clear_map 등 kill 외) 회귀 테스트 보강 여지.

### 🔍 검증 잔여
- 사용자 PIE(차후): 자동 전투 → 퀘스트 진행/완료/수령, 일일 리셋, Q 로그.

### 판정
**블로커 0.** parity·검증 견고 → Codex [4] (값 parity 테스트/보강) 후 Claude 검증 → 머지.
