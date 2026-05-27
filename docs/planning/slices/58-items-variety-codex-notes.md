# PR #58 Codex Notes - Character

## 변경 요약
- `FItemInstance`에 `BaseItemId`, 방어/체력/치명피해 affix 필드를 추가했다. 기존 필드는 0 기본값을 유지해 PR #53/#54 저장 payload와 하위 호환된다.
- `ItemFactory`는 슬롯별 base item 카탈로그를 사용해 `DisplayName = rarity prefix + base item name`, `ItemId = base id + rarity + level` 형태로 생성한다. `FRandomStream` 주입 overload를 추가해 자동화에서 결정적 RNG를 검증한다.
- 세트는 Warrior/Guardian/Arcane에 Assassin/Hunter/Holy/Berserker를 추가했다. 2/4세트 보너스는 client `SetBonusFormula`와 server `setBonus.ts`에 동일하게 반영했다.
- affix pool은 CritRate/AtkSpeed/MagicAtk에 PhysDef/MagicDef/Hp/CritDmg를 추가했다. rarity별 affix 개수는 PR #40 규칙을 유지한다.
- `InventoryComponent::ComputeEquipmentBonus`, `FItemPowerScore`, server `equipment.ts`가 신규 affix를 반영한다.
- `ItemDB.csv`는 슬롯별 base catalog 형태로 정리했고, ko/en HUD localization에 신규 affix/set 라벨을 추가했다.

## 검증 포인트
- Common/None은 affix와 set이 계속 비활성이다.
- Mythic은 expanded affix pool에서 기존 규칙대로 3개만 고른다.
- 세트 보너스는 4-piece에서 2-piece와 추가 4-piece 보너스를 누적한다.
- save round trip은 `BaseItemId`, 신규 set enum, 신규 affix 필드를 그대로 보존한다.

## 실행한 검증
- `npm test -- drop.test.ts setBonus.test.ts equipment.test.ts`

## 남은 검증
- UE `IdleProjectEditor Win64 Development` build
- UE Automation `IdleProject`
