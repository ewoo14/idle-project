퀘스트 시스템 V1 의 퀘스트 로그 UI 를 구현하라. 기획서: docs/planning/slices/18-quest-system-v1.md §2.2. 이미 머지된 character 산출(UQuestService) 사용.

UQuestService 공개 API (client/Source/IdleProject/GameCore/QuestService.h):
- TArray<FQuestState> GetActiveQuestStates() const; // FQuestState: QuestId, Progress, bClaimed 등
- bool GetQuestState(const FString& QuestId, FQuestState& OutState) const;
- FQuestClaimResult ClaimQuest(const FString& QuestId);
- (FQuestDefinition: 제목/Objective/TargetCount/보상 — title·target 표시용)
플레이어/GameInstance 에서 QuestService 접근 경로 확인(UIdleGameInstance 연동되어 있음).

구현:
1. 퀘스트 로그 UI(IdleHUD DrawHUD 확장 또는 위젯): 활성 퀘스트 목록 — 제목 / 진행도(Progress/TargetCount) 바 / 완료 시 "수령" 버튼. 메인/일일 구분 표시.
2. 토글: 단축키 **Q**(GDD §단축키)로 퀘스트 로그 열기/닫기. 수령 버튼 클릭(또는 키) → ClaimQuest(questId) → 보상 반영 + 목록 갱신.
3. V1 단순 패널 + 한글 라벨("퀘스트", "메인", "일일", "진행", "수령", "완료"). design-system 토큰 참고.
4. docs/planning/04-art-direction.md 에 퀘스트 로그 V1 배치/색상 1문단.

제약: 한글 주석/라벨. 퀘스트 로직(QuestService/GameInstance)·서버는 수정 금지(조회·ClaimQuest 호출만). 가능하면 Build.bat + Automation(UI 모델) 검증. push 금지. 커밋 prefix: codex(designer):.
