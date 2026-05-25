# PR #6 보조 4파트 합동 호출

당신은 Codex 보조 4파트 합동 (백엔드 + 밸런스 + 디자이너 + QA) — PR #6 단계 [2] 보조 호출.

## 컨텍스트
- 브랜치: plan/06-auto-battle-v1
- PR: GitHub #6
- 기획서: `docs/planning/slices/06-auto-battle-v1.md`
- 캐릭터 메인 완료: 8 commit (6f0028e~724b741), Build Succeeded, 5 Automation 테스트 통과

## PR #4 학습 사항 (반드시 반영)
- UENUM 에 None=0 / TestEqual float `.0f` / int64 `static_cast<int64>()` 캐스트
- 모듈 루트 기준 include
- BP/.uasset 직접 생성 불가

## 사전 조사
1. git log main..HEAD --oneline | head -12 (캐릭터 메인 8 commit + plan)
2. cat client/Source/IdleProject/CombatSystem/CombatFormulas.h
3. cat client/Source/IdleProject/CombatSystem/CombatComponent.cpp | head -50
4. cat client/Source/IdleProject/CharacterSystem/IdleMonster.cpp | head -50
5. cat client/Source/IdleProject/UI/IdleHUD.h
6. cat client/Source/IdleProject/UI/IdleHUD.cpp
7. cat server/src/core/formulas/level.ts
8. cat docs/planning/05-balance-philosophy.md | head -50
9. cat docs/qa/regression-checklist.md | head -60

## 4파트 작업

### 파트 1 — 백엔드 (codex-backend)
**산출**:
- `server/src/core/formulas/combat.ts` 신규:
  ```typescript
  /**
   * 데미지 계산 — UE5 client CombatFormulas C++ 미러.
   * 최소 보장: ATK × 0.05 (방어가 압도해도 1 이상)
   */
  export function computeDamage(atk: number, def: number): number {
    return Math.max(atk * 0.05, atk - def * 0.6);
  }
  ```
- `server/src/core/formulas/combat.test.ts` — vitest 단위 테스트 ≥ 5건:
  - computeDamage(100, 20) === 88
  - computeDamage(10, 100) === 0.5 (최소 보장)
  - computeDamage(50, 0) === 50
  - computeDamage(0, 0) === 0
  - computeDamage(1000, 500) === 700 (1000 - 300)
- (선택) `core/formulas/index.ts` 에 export 추가
- **검증**: `cd server && npm test` 통과

**커밋**: `codex(backend): combat.ts 클라이언트 미러 + vitest`

### 파트 2 — 밸런스 (codex-balance)
**산출**:
- `docs/planning/05-balance-philosophy.md` 부록에 다음 표 추가:
  - "## 부록 — M1 자동 전투 V1 수치 (PR #6)"
  - Monster 기본 수치 (슬라임): HP 50 / Atk 8 / Def 5 / Reward Gold 10~15
  - 전사 레벨 1 PhysAtk = 24, PhysDef = 16.0 (StatFormulas 결과) 와 비교
  - 1마리 처치 시간 추정: (50 HP) / (24 - 5*0.6 = 21 dmg) ≈ 3 회 공격 → 3 × atkSpeed (=1.0s) = 3초
  - 시간당 처치 수: ~1200 마리 (5 마리 spawn 무한 respawn 가정)
  - 시간당 골드: ~14400~18000 gold
  - 시간당 EXP: monster 1마리 = level × 12 = 12 EXP (PR #1 §3.2.2 미러) → 시간당 ~14400 EXP → 레벨 1→2 (3506 EXP) 약 14분 (M1 단순 환경, 후속 PR 인벤토리/스킬 추가 후 보정)
- `docs/planning/04-art-direction.md` 또는 별도 — Monster 색상 토큰 (`--monster-slime: #5BC07A`) 추가 (선택)

**커밋**: `codex(balance): M1 자동 전투 수치 표 + 시간당 처치/골드/EXP 추정`

### 파트 3 — 디자이너 (codex-designer)
**산출**:
- `client/Source/IdleProject/UI/IdleHUDWidget.h/cpp` 신규 (SCompoundWidget 기반 Slate):
  ```cpp
  class SIdleHUDWidget : public SCompoundWidget {
  public:
    SLATE_BEGIN_ARGS(SIdleHUDWidget) {}
    SLATE_END_ARGS()
    void Construct(const FArguments& InArgs);
    void UpdateHp(float Current, float Max);
    void UpdateExp(int64 Current, int64 Next);
    void UpdateGold(int64 Amount);
    void UpdateLevel(int32 Level);
  private:
    TSharedPtr<class STextBlock> HpText;
    TSharedPtr<class STextBlock> ExpText;
    TSharedPtr<class STextBlock> GoldText;
    TSharedPtr<class STextBlock> LevelText;
  };
  ```
- 위젯 레이아웃 (Slate): 좌상단 VerticalBox (HP / EXP / Level) + 우상단 VerticalBox (Gold)
- 컬러: UIThemeTokens 사용 (Theme::TextPrimary, Theme::AccentRed for HP, Theme::AccentBlue for EXP, Theme::AccentGold for Gold)
- IdleHUD.cpp 갱신: PostInitializeComponents 에서 SNew(SIdleHUDWidget) 생성 후 GEngine->GameViewport->AddViewportWidgetContent 호출
- 델리게이트 콜백 (HandleGoldChanged 등) 에서 RootWidget->UpdateGold 등 호출
- 한글 텍스트: "HP", "EXP", "골드", "Lv."

**커밋**: `codex(designer): IdleHUDWidget Slate UI 실구현 + 테마 토큰 적용`

### 파트 4 — QA (codex-qa)
**산출**:
- `docs/qa/scenarios/M1-auto-battle-v1.md` 신규 (Given/When/Then 한글, ≥ 8건):
  1. PIE 진입 → 캐릭터 + 몬스터 5마리 spawn 확인
  2. 자동 전투 시작 → 캐릭터가 가장 가까운 몬스터 추격
  3. 사거리 도달 → 공격 시작 → 몬스터 HP 감소
  4. 몬스터 HP 0 → 사망 + GoldDrop spawn
  5. GoldDrop 자동 흡수 → Gold 증가 + GoldDrop Destroy
  6. 몬스터 사망 5초 후 동일 위치 재spawn
  7. HUD 의 HP / EXP / Gold 실시간 갱신
  8. 게임 시작 시 NetworkClient guest register 호출 + Output Log 확인
  9. (엣지) 서버 미기동 시 graceful — register 실패해도 게임 진행
  10. (엣지) 모든 몬스터 동시 사망 시 5초 후 일괄 respawn
- `docs/qa/scenarios/README.md` 갱신: M1 자동 전투 항목 추가
- `docs/qa/regression-checklist.md` §1 클라이언트 카테고리에 항목 추가:
  - [ ] 자동 전투 — 캐릭터가 몬스터 자동 추격/공격
  - [ ] 골드 드롭 + 자동 흡수
  - [ ] 몬스터 respawn (5초 후)
  - [ ] HUD HP/EXP/Gold 실시간 갱신
  - [ ] NetworkClient guest register 호출 + 서버 미기동 graceful

**커밋**: `codex(qa): M1 자동 전투 시나리오 + 회귀 §1 갱신`

## 자기 검증 (커밋 전)
- [ ] cd server && npm test 통과 (기존 + 신규 combat 테스트)
- [ ] cd server && npm run build 통과
- [ ] cd server && npx biome check . 통과
- [ ] UE C++ 코드 sanity (include / 타입)
- [ ] 한글 주석 / 한글 문서

## 푸시
모든 commit 후 git push origin plan/06-auto-battle-v1

## 완료 출력
```
## Codex PR #6 보조 4파트 결과
### 백엔드: ...
### 밸런스: ...
### 디자이너: ...
### QA: ...
### 검증: npm test/build/biome / 커밋 / 푸시
```

작업 디렉터리 `C:\game\idle game\repo`, 브랜치 `plan/06-auto-battle-v1`. 이제 시작하세요.
