# PR #47 기획서: 초월 Transcendence V1

> 무한 성장 2차 prestige. 환생(#19/#46)은 영구 포인트를 쌓는
> 1차 prestige이고, 초월은 환생 위의 장기 목표다. 플레이어가 환생을
> 일정 횟수 이상 완료한 뒤 초월하면 더 깊은 리셋을 감수하는 대신 영구
> 글로벌 전투 스탯 배수를 얻는다.

## 1. 목표 / DoD

환생을 5회 이상 한 뒤 초월하면 초월 횟수가 늘고, 영구 글로벌 스탯
배수가 커져 핵심 전투 스탯이 강해진다.

### DoD 검증

1. `CanTranscend`: `RebirthCount >= TranscendRebirthThreshold(5)`.
2. `Transcend`: 성공 시 `TranscendCount++` 후 깊은 리셋을 수행한다.
3. 글로벌 배수: `1 + max(0, count) * 0.25`, 무한 선형 증가.
4. `RefreshDerivedStats`는 HP, 물리/마법 공격, 물리/마법 방어에만 배수를
   적용한다.
5. 초월 0회는 1.0배로 기존 스탯이 불변이다.
6. HUD는 상태, 요구 환생 수, 현재/다음 배수, 초월 횟수, 버튼 상태를
   표시한다.
7. 서버 미러와 클라이언트 공식은 threshold, gate, multiplier 값이 같다.
8. 서버 Vitest, UE 빌드, UE Automation이 통과한다.

## 2. 범위

### 2.1 초월 공식

- `TranscendFormula.h/.cpp`
- `TranscendRebirthThreshold = 5`
- `GetTranscendStatMultiplier(int32 TranscendCount)`
- `CanTranscend(int32 RebirthCount)`

### 2.2 초월 상태와 행위

- `UIdleGameInstance::TranscendCount`
- `CanTranscend()`, `Transcend()`
- `GetTranscendCount()`
- `GetTranscendStatMultiplier()`
- `PreviewTranscendMultiplier()`
- `OnTranscend`

성공 리셋 범위:

- `RebirthCount = 0`
- `RebirthBonusPoints = 0`
- `CharacterLevel = 1`
- `CurrentExp = 0`
- `NextExp = ExpToNext(1)`
- `AvailableStatPoints = 0`
- `AllocatedStats = FPrimaryStats()`
- `Gold = 0`
- `bChapter1BossDefeated = false`
- `StageService->InitializeDefaultStages()`

### 2.3 스탯 적용

`AIdleCharacter::RefreshDerivedStats`에서 패시브 적용 후, Combat 초기화 전
최종 `Derived`에 초월 배수를 적용한다.

배수 적용 대상:

- `Hp`
- `PhysAtk`
- `MagicAtk`
- `PhysDef`
- `MagicDef`

배수 제외 대상:

- `AtkSpeed`
- `CritRate`
- `CritDmg`
- `Dodge`
- `Accuracy`
- 기타 rate 또는 확률 계열 값

### 2.4 HUD

- 초월 패널을 환생 패널 인근에 배치한다.
- locked/ready 상태를 표시한다.
- `RebirthCount / 5` 요구치를 표시한다.
- 현재 배수와 초월 후 배수를 표시한다.
- 초월 횟수를 표시한다.
- `TranscendAction` HitBox는 가능 상태에서만 활성화한다.
- 클릭 시 `UIdleGameInstance::Transcend()`를 호출한다.
- `OnTranscend` 피드백을 표시하고 `EndPlay`에서 해제한다.
- ko/en CSV 키는 `CsvIntegrity`로 검증한다.

### 2.5 서버 미러

- `server/src/core/formulas/transcend.ts`
- `TRANSCEND_REBIRTH_THRESHOLD = 5`
- `getTranscendStatMultiplier(count)`는 `Math.fround`로 C++ float 경계를
  맞춘다.
- `canTranscend(rebirthCount)`는 `rebirthCount >= 5`를 반환한다.

### 2.6 테스트

- 서버 Vitest: count `0/1/4/10`, rebirthCount `4/5/6`, 음수 가드.
- UE Automation: 공식, gate, preview, reset, count 증가.
- UE Automation: count 0 스탯 회귀와 count 1 배수 적용.
- UE Automation: HUD ViewModel과 CSV 키 정합.

## 3. 범위 외

- 초월 전용 재화, 상점, 스킬트리.
- 3차 prestige.
- 초월 보너스 종류 확장.
- 초월 시 환생 보너스 영구 보존.
- 서버 권위 초월 처리.

## 4. 작업 분배

| 파트 | 작업 | 비중 |
| --- | --- | --- |
| character | 공식, GameInstance, 스탯 배수, Automation | 메인 |
| backend | 서버 공식 미러와 parity 테스트 | 보조 |
| designer | HUD 패널, 로컬라이즈, HitBox | 보조 |
| balance | threshold, 배수, 리셋 trade-off 문서화 | 보조 |
| qa | 임계, 리셋, 배수, 회귀 시나리오 | 보조 |

## 5. 워크플로우 v3

1. PM 기획과 PR 생성.
2. Codex 1차 구현.
3. Claude 1차 리뷰.
4. TM 1차 종합 리뷰와 fix 요청.
5. Codex 2차 fix.
6. Claude 2차 리뷰.
7. TM 2차 확인.
8. PM 최종 검증과 merge.

## 6. 리스크

| 리스크 | 완화 |
| --- | --- |
| 깊은 리셋의 손실감이 과함 | 첫 초월부터 25% 영구 배수로 다음 등반 체감 보상 제공 |
| 배수 중복 또는 누락 | `RefreshDerivedStats` 단일 지점에서 최종 Derived에만 적용 |
| rate 계열 밸런스 붕괴 | 공격속도, 치명타, 회피, 명중 계열은 배수 제외 |
| 서버/클라 공식 drift | `Math.fround` 서버 미러와 경계 테스트 유지 |
| 환생 보상 스케일 중복 | 초월 후 `RebirthCount`를 0으로 돌려 #46 스케일을 재시작 |

## 7. 후속

- 초월 전용 재화와 상점.
- 3차 prestige.
- 초월 보너스 종류 확장.
- 서버 권위 초월.
