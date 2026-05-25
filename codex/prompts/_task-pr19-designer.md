환생 V1 의 UI(환생 패널 + 보스 표시)를 구현하라. 기획서: docs/planning/slices/19-boss-rebirth-v1.md §2.3. 이미 머지된 character 산출(UIdleGameInstance 환생 API, 커밋 9cc8509) 사용.

UIdleGameInstance 공개 API (client/Source/IdleProject/GameCore/IdleGameInstance.h 확인):
- bool CanRebirth() const; // 보스격파 && Lv>=100
- (Rebirth 실행 메서드), GetRebirthCount(), GetRebirthBonusPoints(), 보스 격파 플래그 조회.
정확한 시그니처는 헤더에서 확인.

구현:
1. 환생 패널(IdleHUD DrawHUD 확장 또는 위젯): CanRebirth() 면 "환생 가능" 표시 + 현재 환생 횟수 / 보유 보너스 포인트 / 다음 환생 보상(+5 포인트) / **환생 버튼**. 미충족이면 조건 안내(보스 격파/레벨 100).
2. 환생 버튼(클릭 또는 키) → 확인 후 Rebirth() 호출 → 패널/HUD 갱신.
3. 보스 안개 군주 등장/격파 표시(HUD V1 단순 — 보스 HP 바 또는 라벨).
4. V1 단순 패널 + 한글 라벨("환생", "환생 가능", "환생 횟수", "보너스 포인트", "안개 군주"). design-system 토큰.
5. docs/planning/04-art-direction.md 에 환생 패널 V1 배치/색상 1문단.

제약: 한글 주석/라벨. 환생/보스 로직(GameInstance/IdleMonster)·서버는 수정 금지(조회·Rebirth 호출만). 가능하면 Build.bat + Automation(UI 모델) 검증. push 금지. 커밋 prefix: codex(designer):.
