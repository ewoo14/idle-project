오프라인 보상 V1 의 클라이언트(C++) 공식 미러 + 재화 반영을 구현하라. 기획서: docs/planning/slices/16-offline-rewards-v1.md. 서버 공식과 상수 일치(server/src/core/formulas/offline.ts: CAP 12h=43200, EFFICIENCY 0.75, 시간 보너스 곡선/환생 보너스 동일 로직). 기존 패턴: client/Source/IdleProject/GameCore/IdleGameInstance, NetworkClient, StatFormulas.

구현 범위 (이번 character 호출):
1. client/Source/IdleProject/ 에 C++ 오프라인 공식 미러: 예) GameCore/OfflineRewardFormula.h/.cpp — ComputeOfflineRewards(Level, LastSeenUnixSec, NowUnixSec, RebirthCount) → {CappedSeconds, Gold, Exp, TimeBonus}. 서버 offline.ts 와 동일 상수/곡선(정수 반올림).
2. UIdleGameInstance 연동: 시작 시 LastSeen(로컬 세이브 우선, 없으면 미적용) 으로 경과 계산 → 보상 산출 보관(수령 대기). 수령 API: ClaimOfflineRewards() → 기존 AddExp + 골드 가산 경로 재사용 + LastSeen 갱신. 게임 종료/저장 시 LastSeen 기록.
3. (서버 연동은 graceful) NetworkClient 로 /v1/offline/preview·claim 호출 가능하면 사용하되, 서버 없으면 로컬 계산 폴백. V1 은 로컬 계산으로도 동작해야 함.
4. 자동화 테스트(Tests/): C++ ComputeOfflineRewards 순수 로직 — 상한(>12h→12h), 0경과→0, 효율, 곡선/환생, 단조 증가 (서버 offline.test.ts 와 동일 케이스). 기존 CombatTests 패턴, double 통일.

제약: 한글 주석, "로직 C++". 환영 모달 UI(위젯/HUD 표시)는 designer 후속 호출 범위 — character 는 데이터/수령 로직까지. 가능하면 Build.bat + Automation 검증. push 금지. 커밋 prefix: codex(character):.
