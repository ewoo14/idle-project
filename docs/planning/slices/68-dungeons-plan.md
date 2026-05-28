# 던전 시스템 구현 계획 — PR #68

> **실행 방식:** 워크플로우 v3 [2] Codex 명세. character가 클라+서버 미러+테스트 통합, [3] Claude TM 검증. PM 자율. 기존 CP(#49 GetCombatPower)·일일 리셋(#56 QuestService DailyResetDate)·탑(#50 TowerService CP 게이트) 패턴 재사용.

**목표:** 3종 일일 던전(Gold/Exp/Essence), 일일 제한 + CP 비례 특화 재화 보상.

**아키텍처:** `UDungeonService`가 던전별 일일 입장 카운트(UTC `DailyResetDate`) 관리. `TryRunDungeon(Type, CP)` → 입장 가능+CP 충족 시 `FDungeonFormula` 보상 산출 + 1회 차감. GameInstance가 보상 재화 지급(AddGold/AddExp/RuneEssence). 저장 `SaveVersion` 9→10.

**기술 스택:** UE5 C++ + TS/vitest. `Math.fround` parity. 기존 Service/Save 패턴.

---

## 인터페이스 계약

### 1. DungeonTypes.h (또는 DungeonFormula.h 내)
```cpp
UENUM(BlueprintType)
enum class EDungeonType : uint8
{
    None = 0 UMETA(Hidden),
    Gold = 1 UMETA(DisplayName = "Gold"),
    Exp = 2 UMETA(DisplayName = "Exp"),
    Essence = 3 UMETA(DisplayName = "Essence")
};

USTRUCT(BlueprintType)
struct FDungeonRunResult
{
    GENERATED_BODY()
    UPROPERTY() bool bSuccess = false;
    UPROPERTY() EDungeonType Type = EDungeonType::None;
    UPROPERTY() int64 GoldReward = 0;
    UPROPERTY() int64 ExpReward = 0;
    UPROPERTY() int64 EssenceReward = 0;
};
```

### 2. FDungeonFormula
```cpp
struct IDLEPROJECT_API FDungeonFormula
{
    static constexpr int32 DailyEntryLimit = 3; // 각 던전 일일 3회

    static int32 GetDailyEntryLimit(EDungeonType Type); // 기본 3
    static int64 GetMinimumCp(EDungeonType Type);       // 입장 최소 CP(0 가능)
    // CP 비례 특화 보상 (해당 던전 재화만 > 0)
    static FDungeonRunResult GetRewardForCp(EDungeonType Type, int64 CombatPower);
    //  Gold:  GoldReward = base_gold * f(CP)
    //  Exp:   ExpReward = base_exp * f(CP)
    //  Essence: EssenceReward = base_essence * f(CP)  (룬 #61 에센스)
    //  f(CP) 예: max(1, CP / scale) 선형 또는 sqrt — balance 튜닝, int64 포화 클램프
};
```
- 보상 가이드(balance 조정): Gold base 큰 편(골드 싱크 많음), Exp 레벨 진행 보조, Essence 룬 강화 보조. 일일 3회 × 보상 = 의미있되 경제 교란 없게.

### 3. UDungeonService
```cpp
UCLASS()
class IDLEPROJECT_API UDungeonService : public UObject
{
    GENERATED_BODY()
public:
    // 입장 가능(횟수 남음 + CP≥최소) → 보상 산출 + 입장 1회 차감. 실패 시 bSuccess=false(차감 없음).
    FDungeonRunResult TryRunDungeon(EDungeonType Type, int64 CombatPower, const FString& TodayUtc);
    int32 GetRemainingEntries(EDungeonType Type, const FString& TodayUtc) const;
    void EnsureDailyReset(const FString& TodayUtc); // DailyResetDate != Today면 카운트 0 리셋

    void CaptureState(TMap<EDungeonType,int32>& OutEntriesUsed, FString& OutDailyReset) const;
    void RestoreState(const TMap<EDungeonType,int32>& InEntriesUsed, const FString& InDailyReset);
private:
    UPROPERTY() TMap<EDungeonType, int32> EntriesUsed; // 던전별 오늘 사용 횟수
    UPROPERTY() FString DailyResetDate;                // UTC yyyy-mm-dd
};
```

### 4. IdleGameInstance 연동
```cpp
UPROPERTY(Transient) TObjectPtr<UDungeonService> DungeonService;
void EnsureDungeonService();
UFUNCTION(BlueprintPure) UDungeonService* GetDungeonService() const { return DungeonService; }

// 던전 실행: 캐릭터 CP + 오늘 UTC로 TryRunDungeon → 보상 재화 지급
UFUNCTION(BlueprintCallable, Category="Idle|Dungeon")
FDungeonRunResult TryRunDungeon(EDungeonType Type);
//  내부: CP = 캐릭터 GetCombatPower(), Today = UTC 날짜. 결과 bSuccess면 Gold→AddGold/Exp→AddExp/Essence→RuneEssence 지급(오버플로 포화). autosave.
```
일일 리셋 날짜는 #56 QuestService와 동일 UTC yyyy-mm-dd 포맷 헬퍼 재사용.

### 5. IdleSaveGame.h
```cpp
UPROPERTY() int32 SaveVersion = 10; // 9 → 10
UPROPERTY() TArray<int32> DungeonEntriesUsed; // 또는 TMap 직렬화(EDungeonType→int). 길이 3(Gold/Exp/Essence)
UPROPERTY() FString DungeonDailyResetDate;
```
v<10 마이그레이션: 빈 카운트/리셋(회귀안전).

### 6. server dungeon.ts
```ts
export const DUNGEON_DAILY_ENTRY_LIMIT = 3;
export function getDailyEntryLimit(type: number): number;
export function getMinimumCp(type: number): number;
export function getDungeonReward(type: number, combatPower: number): { gold: number; exp: number; essence: number }; // Math.fround/정수
```
보상/제한 클라 동일.

---

## 테스트 케이스

### 클라 Automation
- TryRunDungeon Gold: CP 충족 → GoldReward>0, Exp/Essence 0, 입장 1 차감
- 일일 제한: 3회 후 4번째 → bSuccess=false(차감 없음)
- 일일 리셋: DailyResetDate 다음날 → EnsureDailyReset 카운트 0
- CP 게이트: CP < GetMinimumCp → false
- GetRemainingEntries 정확(3→2→1→0)
- GameInstance TryRunDungeon → 재화 지급(AddGold/AddExp/RuneEssence) 확인
- Capture→Restore 라운드트립(EntriesUsed/DailyResetDate)
- v9 세이브(던전 없음) → 빈 카운트/리셋 회귀안전

### 서버 vitest
- getDungeonReward 각 타입 CP 앵커(Gold/Exp/Essence만 > 0), Math.fround
- getDailyEntryLimit===3, getMinimumCp 타입별

---

## Codex 작업 분배
| 파트 | 작업 |
| --- | --- |
| character (메인) | EDungeonType/FDungeonFormula/UDungeonService + GameInstance 연동(재화 지급) + 저장 v10 + 서버 dungeon.ts + Tests |
| designer | 던전 패널 HUD(3종/잔여/보상 미리보기) + 로컬라이즈 ko/en |
| balance | 던전 보상/CP/일일제한 → 재화 수급 페이싱 시뮬 + 경제 가드 |
| (backend/qa) | character 흡수, [3] Claude TM parity·커버리지 |

## 워크플로우 v3
[1] ✅ 기획·계획 + PR → [2] character(+designer/balance) → [3] Claude TM → [4] fix(필요시) → [5] 검증 → [N] CI 그린 + 머지

## Self-Review
- DoD 1~8 매핑 ✅
- placeholder 없음(보상 가이드 명시, 정밀 수치 balance 위임)
- 타입 일관성: `EDungeonType`(3)/`FDungeonFormula`/`UDungeonService`/`FDungeonRunResult`/`SaveVersion`(10)/`dungeon.ts` 전 섹션 일치
- 주의: 일일 리셋 #56 UTC 패턴 재사용 / 입장 3중 가드 1회 차감 / 보상 int64 포화 / 클라↔서버 parity / 경제 페이싱 가드(balance)
