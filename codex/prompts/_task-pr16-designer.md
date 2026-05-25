오프라인 보상 V1 의 환영 모달 UI 를 구현하라. 기획서: docs/planning/slices/16-offline-rewards-v1.md §2.3. 이미 머지된 character 산출(UIdleGameInstance 오프라인 API)을 사용한다.

UIdleGameInstance 공개 API (client/Source/IdleProject/GameCore/IdleGameInstance.h):
- FOfflineRewardResult PreviewOfflineRewards(int64 NowUnixSec, int32 RebirthCount=0) const; // {CappedSeconds, Gold, Exp, TimeBonus}
- FOfflineRewardResult ClaimOfflineRewards();  // 수령 → 재화 반영 + LastSeen 갱신
- int64 GetLastSeenUnixSec() const;
(FOfflineRewardResult 정의: client/Source/IdleProject/GameCore/OfflineRewardFormula.h)

구현:
1. 게임 시작(플레이어 BeginPlay 또는 HUD 초기화) 시 PreviewOfflineRewards(현재 Unix 시각) 호출 → Gold/Exp > 0 이면 **환영 모달** 표시.
2. 모달 내용: 오프라인 경과 시간(CappedSeconds 를 시:분 포맷), 획득 골드, 획득 EXP, **수령 버튼**.
3. 수령 → ClaimOfflineRewards() 호출 → 재화 반영 + 모달 닫기. (Gold/Exp 0 이면 모달 미표시.)
4. 표현: 기존 UI 패턴 따름 — client/Source/IdleProject/UI/IdleHUD (C++ DrawHUD) 확장 또는 UMG 위젯. V1 은 단순 패널 + 수령(클릭 또는 지정 키) 허용. design-system 토큰(docs/planning/ui-tokens.json) 참고, 한글 라벨("오프라인 보상", "경과 시간", "획득 골드/EXP", "수령").
5. docs/planning/04-art-direction.md 에 환영 모달 V1 배치/색상 1문단 추가.

제약: 한글 주석/라벨. 오프라인 공식/GameInstance 로직(character 산출)은 수정 금지(조회·수령 호출만). 서버/밸런스/테스트 로직은 범위 외. 가능하면 Build.bat 컴파일 확인. push 금지. 커밋 prefix: codex(designer):.
