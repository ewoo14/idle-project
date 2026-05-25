# PR #9 보조 4파트 합동 호출

당신은 Codex 보조 4파트 합동 (백엔드 + 밸런스 + 디자이너 + QA) — PR #9 단계 [2] 보조.

## 컨텍스트
- 브랜치: plan/09-inventory-v1
- PR: GitHub #9
- 기획서: `docs/planning/slices/09-inventory-v1.md`
- 캐릭터 메인 완료: 6 commit (864b886~d3bbda3), Build Succeeded, 5 Automation Inventory 테스트 통과

## PR #4~#8 학습 사항 (반드시 반영)
- UENUM None=0 / TestEqual float `.0f` / int64 `static_cast<int64>()` 캐스트
- 모듈 루트 기준 include
- BP/.uasset 직접 생성 불가 (C++ + Slate 만)
- UE 5.7 SkyAtmosphere 모듈 의존성 추가 필요 — 본 PR 무관

## 사전 조사
1. git log main..HEAD --oneline | head -10
2. cat client/Source/IdleProject/ItemSystem/ItemTypes.h
3. cat client/Source/IdleProject/ItemSystem/InventoryComponent.h
4. cat client/Source/IdleProject/ItemSystem/ItemFactory.cpp | head -80
5. cat client/Source/IdleProject/UI/IdleHUDWidget.h
6. cat client/Source/IdleProject/UI/IdleHUDWidget.cpp | head -50
7. cat server/src/core/formulas/index.ts
8. ls server/src/core/formulas/

## 4파트 작업

### 파트 1 — 백엔드 (codex-backend)
**산출**:
- `server/src/core/formulas/equipment.ts` 신규:
  ```typescript
  /**
   * 장비 PowerScore — UE5 client FItemPowerScore::Compute 미러.
   * (Atk + Def + Hp/10) × (1 + EnhanceLevel × 0.1)
   */
  export type ItemSlot = 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8;
  // 1=Weapon, 2=Helmet, 3=Top, 4=Bottom, 5=Shoes, 6=Gloves, 7=Cloak, 8=Accessory
  
  export type ItemRarity = 1 | 2 | 3;
  // 1=Common, 2=Uncommon, 3=Rare (PR #9 범위)
  
  export interface ItemInstance {
    itemId: string;
    slot: ItemSlot;
    rarity: ItemRarity;
    bonusAtk: number;
    bonusDef: number;
    bonusHp: number;
    enhanceLevel: number;
  }
  
  export function computeItemPowerScore(item: ItemInstance): number {
    return (item.bonusAtk + item.bonusDef + item.bonusHp / 10) * (1 + item.enhanceLevel * 0.1);
  }
  
  export interface EquipmentBonus {
    bonusAtk: number;
    bonusDef: number;
    bonusHp: number;
  }
  
  export function computeInventoryBonus(equippedItems: ItemInstance[]): EquipmentBonus {
    return equippedItems.reduce(
      (acc, item) => ({
        bonusAtk: acc.bonusAtk + item.bonusAtk * (1 + item.enhanceLevel * 0.1),
        bonusDef: acc.bonusDef + item.bonusDef * (1 + item.enhanceLevel * 0.1),
        bonusHp: acc.bonusHp + item.bonusHp * (1 + item.enhanceLevel * 0.1),
      }),
      { bonusAtk: 0, bonusDef: 0, bonusHp: 0 },
    );
  }
  ```
- `server/src/core/formulas/equipment.test.ts` 신규 vitest ≥ 6건:
  - computeItemPowerScore 결정성 + 강화 보너스
  - computeInventoryBonus 합계
  - 빈 array → 0
  - Slot/Rarity 타입 안전
- `index.ts` export 추가
- **검증**: cd server && npm test 통과

**커밋**: `codex(backend): equipment.ts 미러 + vitest`

### 파트 2 — 밸런스 (codex-balance)
**산출**:
- `client/Content/Data/ItemDB.csv` 신규 (24행 기본 템플릿):
  - 8슬롯 × 3등급 = 24행
  - 컬럼: ItemId, NameKr, Slot (1~8), Rarity (1~3), BaseAtk, BaseDef, BaseHp, MaxEnhance
  - 등급별 기본값:
    - Common: ATK 3~5, DEF 2~4, HP 10~20, MaxEnhance 5
    - Uncommon: ATK 6~10, DEF 5~8, HP 25~40, MaxEnhance 8
    - Rare: ATK 11~20, DEF 9~15, HP 45~80, MaxEnhance 10
  - 슬롯별 가중 (Weapon 100% ATK, 방어구 70% DEF + 30% HP, Accessory 균형)
- 한글 이름 예: "낡은 검", "정련된 검", "강철 검" (등급 prefix + 슬롯)
- `docs/planning/05-balance-philosophy.md` 부록 "M2 인벤토리 V1 수치" 추가:
  - 등급별 ATK/DEF/HP 평균 표
  - PowerScore 공식
  - 자동 장착 점수 비교 흐름
  - 1마리 처치 시 5% 드롭 → 1시간 ~60개 드롭 추정 (현 슬라임 환경 1200마리)

**커밋**: `codex(balance): ItemDB.csv 24행 + balance 부록`

### 파트 3 — 디자이너 (codex-designer)
**산출**:
- `client/Source/IdleProject/UI/IdleHUDWidget.h/cpp` 갱신:
  - SIdleHUDWidget 에 새 멤버 `TSharedPtr<STextBlock> EquipmentText` (좌하단)
  - 새 메서드 `void UpdateEquipment(const FText& WeaponName, const FText& ArmorSummary)`
  - 슬롯 등급 색상 매핑 (UIThemeTokens 참조 — Common/Uncommon/Rare)
  - IdleHUD.cpp 에서 InventoryComponent.OnEquippedChanged 콜백 시 UpdateEquipment 호출
- 텍스트 형식 (한글):
  - 무기: `"⚔ 강철 검 (Rare, ATK+15)"`
  - 방어구 합: `"🛡 방어구 5/7 슬롯 (DEF+25, HP+120)"`
- UIThemeTokens 갱신 (필요 시): Common/Uncommon/Rare 색상이 이미 정의됐다면 그대로

**커밋**: `codex(designer): HUD 좌하단 장비 요약 + 등급 색상`

### 파트 4 — QA (codex-qa)
**산출**:
- `docs/qa/scenarios/M2-inventory-v1.md` 신규 (Given/When/Then 한글 ≥ 8건):
  1. PIE 진입 → 인벤토리 빈 상태 (Items.Num == 0)
  2. 슬라임 사망 (5번 반복) → 평균 0~1 장비 드롭 spawn
  3. 드롭된 장비 자동 흡수 → 인벤토리 추가
  4. Common 무기 → Uncommon 무기 드롭 → 자동 장착 + HUD 갱신
  5. 두 슬롯 (Weapon + Helmet) 동시 장착 시 ComputeEquipmentBonus 합
  6. 강화 +1 적용 시 PowerScore 10% 증가
  7. (엣지) 같은 등급 + 더 낮은 PowerScore → 자동 장착 안 됨
  8. (엣지) 최대 100칸 도달 시 가장 약한 아이템부터 자동 폐기 (선택 — 본 PR 범위 외면 후속 PR 명시)
- 자동화 분류 매트릭스 추가
- `docs/qa/scenarios/README.md` 갱신 (M2 인덱스)
- `docs/qa/regression-checklist.md` §1 클라이언트 + §4 데이터 콘텐츠 항목 추가:
  - [ ] 장비 드롭 5% 확률 동작
  - [ ] 자동 흡수 + 자동 장착
  - [ ] HUD 좌하단 장비 표시
  - [ ] ComputeEquipmentBonus 합산 정합
  - [ ] PowerScore 강화 계수 적용

**커밋**: `codex(qa): M2 인벤토리 시나리오 + 회귀 §1/§4`

## 자기 검증 (커밋 전)
- cd server && npm test (기존 + 신규 equipment)
- cd server && npm run build
- cd server && npx biome check .
- UE 코드 변경 시 UE Build.bat 재실행 (가능 시)

## 푸시
모든 commit 후 git push origin plan/09-inventory-v1

## 완료 출력
```
## Codex PR #9 보조 4파트 결과
### 백엔드 / 밸런스 / 디자이너 / QA 각 커밋 + 산출 요약
### 검증
```

작업 디렉터리 `C:\game\idle game\repo`, 브랜치 `plan/09-inventory-v1`. 이제 시작하세요.
