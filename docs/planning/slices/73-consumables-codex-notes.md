# PR #73 Codex Character 구현 노트

- `EConsumableType` 6종과 `FConsumableFormula`/`consumable.ts` 상수를 `AttackTonic=0`부터 `WisdomBooster=5`까지 고정했다.
- `UBuffService`가 보유 수량과 `BuffEndUnixSec`를 단일 소유하며, 사용 성공 시 `Now + 1800`으로 종료 시각을 갱신한다.
- 스탯 버프는 `RefreshDerivedStats`에서 한 번만 적용한다. 공격 물약은 물공/마공, 방어 물약은 HP/물방/마방, 만능 영약은 5개 코어 파생 스탯 전체에 적용한다.
- 경제 버프는 `AddGold`, `AddExp`, `ApplyEquippedPetDropBonusChance` 경로에서 한 번만 적용한다. EXP는 #72 교훈에 맞춰 `AddExp`가 단일 적용 지점이다.
- 저장은 `SaveVersion=14`와 `Consumables` 배열로 확장했다. v13 이하는 빈 소비 상태로 마이그레이션하며, 환생/초월은 소비 보유/활성 버프를 초기화하지 않는다.
- 검증: `IdleProject.Consumable`, `IdleProject.GameCore.SaveSystem`, 서버 `npm run lint`, `npm run test -- consumable save`, `npm run build`.
