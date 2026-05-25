# M1 클라이언트 코어 부트 QA 시나리오

PR #4 `plan/04-client-core-boot` 범위의 UE5 5.7 클라이언트 수동/자동 검증 시나리오다.

## 1. 프로젝트 열기 및 컴파일

- 도구: 수동 PIE, UE Editor Compile
- Given UE 5.7이 설치되어 있고 `client/IdleProject.uproject`가 존재한다.
- When UE Editor에서 `.uproject`를 열고 C++ 컴파일을 실행한다.
- Then 프로젝트가 로드되고 컴파일 에러가 없어야 한다.

## 2. 메인 메뉴 시작 버튼

- 도구: 수동 PIE, log 검증
- Given `W_MainMenu` Blueprint가 `UMainMenuController`를 베이스로 연결되어 있다.
- When PIE를 실행하고 메인 메뉴에서 "시작" 버튼을 클릭한다.
- Then `StartGame()`이 호출되고 `Game` 레벨로 전환되어야 한다.

## 3. 캐릭터 등장

- 도구: 수동 PIE
- Given `Game` 레벨의 기본 Pawn 또는 배치 액터가 `AIdleCharacter`를 사용한다.
- When 인게임에 진입한다.
- Then `AIdleCharacter`가 화면 중앙 또는 좌측 플레이 영역에 보여야 한다.

## 4. 좌우 이동 입력

- 도구: 수동 PIE
- Given 인게임에서 플레이어가 `AIdleCharacter`를 조작 중이다.
- When `A`/`D` 또는 `←`/`→` 키를 누른다.
- Then 캐릭터가 좌우 방향으로 이동해야 한다.

## 5. 공격 입력

- 도구: 수동 PIE, log 검증
- Given 인게임에서 플레이어가 `AIdleCharacter`를 조작 중이다.
- When `Space` 키를 누른다.
- Then 공격 액션이 트리거되고 임시 모션 또는 Output Log 메시지가 확인되어야 한다.

## 6. Esc 메뉴 입력

- 도구: 수동 PIE, log 검증
- Given 인게임이 진행 중이다.
- When `Esc` 키를 누른다.
- Then 일시정지 또는 메뉴 전환 훅이 호출되어야 한다.

## 7. StatFormulas 로그 검증

- 도구: log 검증
- Given 게임 시작 시 전사 레벨 1의 1차/2차 능력치를 계산한다.
- When Output Log에서 `StatFormulas` 결과를 확인한다.
- Then 전사 레벨 1 derived stats가 기대값과 일치해야 한다. 기준값은 HP 120, PhysAtk 24, Accuracy 0.762다.

## 8. UE Automation 수식 테스트

- 도구: UE Automation
- Given `client/Source/IdleProject/Tests/` 아래 Automation 테스트가 빌드에 포함되어 있다.
- When `Automation RunTests IdleProject`를 실행한다.
- Then `StatFormulas`와 `LevelFormulas` 테스트가 모두 통과해야 한다.

## Level cross-validation 결과

서버 `server/src/core/formulas/level.ts`와 클라이언트 `client/Content/Data/LevelCurveDB.csv`의 `ExpToNext` 앵커 비교 결과다.

| Level | server (ExpToNext) | client (LevelFormulas CSV) | diff |
| --- | ---: | ---: | ---: |
| 1 | 150 | 150 | 0 |
| 10 | 3,506 | 3,506 | 0 |
| 50 | 43,656 | 43,656 | 0 |
| 100 | 260,594 | 260,594 | 0 |
| 200 | 832,291 | 832,291 | 0 |
