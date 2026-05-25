펫 2종 + 시즌 패스 베타 V1 의 UI 를 구현하라. 기획서: docs/planning/slices/22-pets-season-pass-v1.md §2.2. 이미 머지된 character 산출(펫/시즌 서비스 API, GameInstance 연동) 사용 — 헤더에서 정확한 시그니처 확인(GetEquippedPet*BonusPercent, 펫 장착, 시즌 토큰/티어/ClaimSeasonReward 등).

구현:
1. 펫 UI(IdleHUD DrawHUD 확장 또는 위젯): 펫 2종(강아지 골드+/새 드롭+) 목록 + 장착 버튼 + 현재 장착 표시 + 보너스 % 표기. 한글 라벨.
2. 시즌 패스 UI: 무료 트랙 티어 진행 바(현재 시즌 토큰/다음 티어 요구), 티어별 보상 표시, 도달 티어 "수령" 버튼 → ClaimSeasonReward.
3. 진입점: 단축키로 펫/시즌 패널 토글(기존 HUD 단축키 컨벤션 참고). 한글 라벨("펫", "시즌 패스", "장착", "수령", "티어").
4. docs/planning/04-art-direction.md 에 펫/시즌 패스 UI V1 1문단.

제약: 한글 주석/라벨. 펫/시즌 로직(서비스/GameInstance)·서버는 조회·장착·Claim 호출만(로직 변경 금지). 가능하면 Build.bat + Automation(UI 모델) 검증. push 금지. 커밋 prefix: codex(designer):.
