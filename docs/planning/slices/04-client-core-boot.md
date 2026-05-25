# PR #4 기획서 — UE5 클라이언트 코어 부트 (원 슬라이스 ID: S2, M1)

> **순서 복귀**: PR #2 머지 (백엔드 V1) 완료 + 사용자 UE5 **5.7** 설치 완료 (`C:\Program Files\Epic Games\UE_5.7\`) 로 원 마일스톤 순서 복귀. 본 PR 이 M1 첫 슬라이스. 메모리 `[[project-pr-order]]` 추적.

---

## 1. 목표 / DoD

UE5 5.7 에디터에서 다음이 동작:
1. `client/IdleProject.uproject` 더블 클릭 → UE5 5.7 에디터 열림
2. 메인 메뉴 (시작 / 설정 / 종료) — UMG 위젯
3. "시작" → 인게임 레벨로 전환, **전사 캐릭터 1종** 이 횡스크롤 카메라 안에 등장
4. 캐릭터: 이동 (좌/우 키) + 공격 (스페이스) 기본 모션 — 임시 메시 / Paper2D 큐브 OK
5. C++ `Source/IdleProject/` 모듈 구조 확립 + `StatFormulas.cpp` (서버 TS 미러)
6. 데이터 테이블 (`ClassDB`, `LevelCurveDB`) — CSV 임포트 → 게임 인스턴스 로드
7. (선택) NetworkClient 최소 구현 — `/v1/auth/register` 호출 가능 (PR #5 자동 전투 슬라이스 직전까지 read-only)

DoD 검증 시나리오:
- UE5 5.7 에디터에서 프로젝트 열기 → 컴파일 통과
- 메인 메뉴 → "시작" 클릭 → 인게임 전환
- 캐릭터가 화면 중앙 + 키보드 입력으로 좌/우 이동, 스페이스 키 공격 모션
- `Output Log` 에 `StatFormulas` 호출 결과 출력 (예: 전사 레벨 1 의 derived stats)

## 2. 범위 (In Scope)

### 2.1 UE5 프로젝트 스캐폴드
- `client/IdleProject.uproject` (UE 5.7, C++ 모듈)
- `client/Config/` — DefaultEngine.ini, DefaultGame.ini, DefaultInput.ini
- `client/Source/IdleProject/`:
  - `IdleProject.h/cpp` — 모듈 엔트리
  - `IdleProject.Build.cs`
  - `IdleProjectGameModule.cpp`
- `client/Source/IdleProject.Target.cs`, `IdleProjectEditor.Target.cs`
- `client/IdleProject.uproject` 의 modules 등록

### 2.2 핵심 모듈 (Source/IdleProject/)
- `GameCore/` — `UIdleGameInstance.h/cpp` (세이브 슬롯, 글로벌 서비스)
- `CharacterSystem/`:
  - `AIdleCharacter.h/cpp` — Pawn (Paper2D 또는 Capsule + 임시 메시)
  - `StatFormulas.h/cpp` — 서버 `core/formulas/stats.ts` C++ 미러 (결정적 함수)
  - `LevelFormulas.h/cpp` — 서버 `core/formulas/level.ts` C++ 미러
- `UI/`:
  - `W_MainMenu.uasset` (Blueprint Widget 또는 C++ Base + BP 표현)
  - `MainMenuController.h/cpp` (C++)
- `DataAssets/`:
  - `ClassDB.csv` (5직업 — PR #2 서버 정의와 1:1)
  - `LevelCurveDB.csv` (레벨 1~200 EXP — 공식 결과)
- `Tests/`:
  - `StatFormulasTests.cpp` — UE Automation 테스트 (전사 레벨 1/10/50/100 derived stats 가 서버 결과와 1:1)

### 2.3 입력 / 카메라
- `DefaultInput.ini` — `MoveLeft` (A/←), `MoveRight` (D/→), `Attack` (Space), `MenuToggle` (Esc)
- 카메라: 횡스크롤 사이드뷰 (`SpringArm` + `CameraComponent`, Y 축 카메라)

### 2.4 클라이언트 ↔ 서버 미러 검증
- `StatFormulas.cpp` 의 함수가 `server/src/core/formulas/stats.ts` 와 **동일 결과** 검증.
- 검증 방식: UE Automation 테스트 + 별도 `tools/cross-validation/` 스크립트 (server JSON 결과 ↔ UE 로그 결과 diff)
- 본 PR 에서는 UE Automation 테스트만 (자동화는 후속 PR)

### 2.5 NetworkClient (최소)
- `Source/IdleProject/NetworkClient/UApiClient.h/cpp`
  - `Get<T>(Path)`, `Post<T>(Path, Body)` 기본 메서드
  - JWT 토큰 자동 헤더 첨부 (메모리 보관, 1.0 까지 디스크 저장 미루기)
  - 본 PR 은 호출 사용 없음 — 골격만. PR #5 자동 전투 직전 PR 에서 register/login 실 호출 시작.

### 2.6 문서
- `docs/planning/05-balance-philosophy.md` 갱신 — "공식 미러: 서버 TS + UE5 C++" 양방향 표 추가
- `docs/api/character.md` 갱신 — 클라이언트 측 미러 위치 명시
- `client/README.md` — 빌드 / 실행 / 테스트 명령 보강 (UE 5.7 정확한 경로)
- `docs/qa/scenarios/M1-client-core-boot.md` — 신규 시나리오 (5~8건)

### 2.7 CI 갱신
- `.github/workflows/client-ci.yml` — `build/client` 라벨 트리거 시 UE5 5.7 헤드리스 빌드:
  - **현실적 한계**: GitHub-hosted runner 에 UE5 미설치. **자체 호스팅 러너 필요**.
  - 본 PR 단계는 self-hosted runner placeholder 만. 실제 활성화는 self-hosted 러너 셋업 후.
  - 대안: 본 PR 의 client/ 빌드를 PR 본문에 PM 이 로컬 빌드 결과 첨부 (PR 머지 전 의무).

## 3. 범위 외 (Out of Scope)

| 항목 | 시점 |
| --- | --- |
| 자동 전투 AI | PR #5 (M1-S3) |
| 몬스터 / 사냥 | PR #5 |
| 인벤토리 / 장비 UI | PR #6 (M2-S4) |
| 스킬 트리 | PR #7 (M2-S5) |
| 오프라인 보상 | PR #8 (M3-S6) |
| 백엔드 실 호출 | PR #5 직전 (등록/로그인부터 시작) |
| 다국어 (i18n) | 1.0 |
| 다직업 (전사 외) | PR #11 |
| 펫 / 시즌 패스 | PR #12 |
| Steam SDK | PR #13 (M6) |
| 사운드 | M5 이후 |
| Niagara 이펙트 | PR #5 (전투 시작 시점)에서 첫 도입 |

## 4. 7파트 작업 분배 — Codex 호출 계획

| 파트 | 작업 | Codex 호출 |
| --- | --- | --- |
| **백엔드·DB** | (N/A — 본 PR 은 클라이언트 슬라이스) | ❌ |
| **캐릭터·아이템·능력치 (메인)** | UE5 프로젝트 스캐폴드 + C++ 모듈 + StatFormulas / LevelFormulas (서버 TS 미러) + AIdleCharacter | ✅ 1회 (메인) |
| **밸런스** | LevelCurveDB.csv (PR #2 의 expToNext 결과 1:1) + UE Automation 검증 테스트 | ✅ |
| **디자이너** | W_MainMenu UMG + 카메라 / 입력 / 캐릭터 임시 메시 (Paper2D 또는 큐브) + UI 토큰 적용 | ✅ |
| **QA** | docs/qa/scenarios/M1-client-core-boot.md + UE Automation 테스트 + 회귀 §1 갱신 | ✅ |
| **스토리** | (N/A — PR #8 첫 기여 — 메인 메뉴의 게임 제목 텍스트 정도만 placeholder) | ❌ |
| **퀘스트** | (N/A) | ❌ |

→ 총 Codex 실호출 **4회** (캐릭터 메인 + 밸런스 + 디자이너 + QA)

## 5. 7파트 Codex 호출 순서

1. **캐릭터 (메인)** — UE5 프로젝트 스캐폴드 + 모듈 + StatFormulas + AIdleCharacter (가장 큰 단위)
2. **밸런스** — LevelCurveDB.csv 생성 (server 결과 기반) + Automation 테스트
3. **디자이너** — UMG + 카메라 + 입력 + 임시 메시 (캐릭터 위에 얹기)
4. **QA** — 시나리오 + Automation 보강

## 6. 워크플로우 v2 (PR #2 학습 사항 반영)

- 단계 [3b]/[6b] **Codex TM 프롬프트 단순화** — "11 항목 ✅/❌ 표" 위주, 사전 조사 시간 최소화.
- 단계 [5] Codex 2차 fix 가 큰 작업이면 분할 가능 (예: 블로커 별도 호출, 권장 별도 호출).
- 본 PR 의 UE5 빌드 검증은 **PM 이 직접 로컬에서 1회 빌드 + 스크린샷** (시간 ~15분) 첨부 의무.

## 7. 일정 (잠정)

- 단계 [1]: 본 문서 (완료)
- 단계 [2]: 4 Codex 호출 (캐릭터 메인 ~30~50분, 보조 3 ~ 15~20분 합) → 총 60~90분
- 단계 [3a/3b]: 각 10~15분
- 단계 [4]~[7]: 30분
- 단계 [8]: PM 로컬 빌드 + 종합 소견 + 머지 → 30분
- **총: 약 3시간**

## 8. 리스크

| 항목 | 위험 | 대응 |
| --- | --- | --- |
| Codex 가 UE5 프로젝트 파일 (.uproject, .uasset) 생성 시 형식 오류 | 높음 | 텍스트 (.uproject = JSON, .Build.cs = C#) 위주만 Codex 가 작성, .uasset 은 빈 placeholder 또는 PR 머지 후 PM 이 에디터에서 생성 |
| UE5 5.7 자체가 너무 신버전이라 Codex 가 5.4 기준으로 코드 작성 | 중 | 프롬프트에 "UE 5.7" 명시 + UPROPERTY / 모듈 패턴은 5.4~5.7 호환 |
| client-ci 가 self-hosted runner 부재로 빌드 검증 불가 | 중 | PM 로컬 빌드 + 스크린샷 필수, CI 는 코드만 검증 (clang-format 등) |
| StatFormulas 가 서버 TS 와 결과 어긋남 | 중 | UE Automation 테스트에서 server 의 expToNext(100) 결과 ≈ 261,000 등 5~10건 cross-validation |

## 9. 후속 PR 예고

- **PR #5 (M1-S3)** — 자동 전투 V1 (AI, 몬스터, 골드 드롭) + NetworkClient 첫 호출 (register/login)
- **PR #6 (M2-S4)** — 인벤토리 + 장비 V1
- **PR #7 (M2-S5)** — 스킬 트리 V1

---

본 PR 머지 후 `[[project-pr-order]]` 갱신 + PR #5 진입.

## 부록 — LevelCurveDB cross-validation

서버 `server/src/core/formulas/level.ts`의 `expToNext`와 클라이언트 `client/Content/Data/LevelCurveDB.csv`를 2026-05-25 기준으로 수동 비교했다. `LevelFormulas.cpp`는 같은 앵커를 UE Automation으로 검증한다.

| Level | server (ExpToNext) | client (LevelFormulas CSV) | diff |
| --- | ---: | ---: | ---: |
| 1 | 150 | 150 | 0 |
| 10 | 3,506 | 3,506 | 0 |
| 50 | 43,656 | 43,656 | 0 |
| 100 | 260,594 | 260,594 | 0 |
| 200 | 832,291 | 832,291 | 0 |
