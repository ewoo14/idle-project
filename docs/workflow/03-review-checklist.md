# 파트별 리뷰 체크리스트

> 7개 Claude 리뷰 에이전트가 각자 파트별로 사용하는 체크리스트. TM 은 7파트 종합 시 본 항목을 기준으로 누락 여부를 점검.

---

## 공통 (모든 파트)

- [ ] PR 본문 7개 섹션 모두 작성됨
- [ ] 워크플로우 체크리스트 단계 표기 일치
- [ ] 변경 파일이 PR 설명과 일치
- [ ] 시크릿 / 비밀 키 커밋 없음
- [ ] 한글 표기 일관 (외래어 표기법 준수)
- [ ] 문서 갱신이 필요했다면 함께 반영됨

---

## 1. 디자인 (UI/UX, 아트)

- [ ] `docs/planning/04-art-direction.md` 와 정합
- [ ] 컬러 토큰 사용 (하드코딩 금지)
- [ ] 반응형 — 1080p / 1440p / 4K 에서 깨지지 않음
- [ ] 접근성 — 색약, 글자 크기, 키보드 네비
- [ ] 에셋 LFS 추적 (`.gitattributes`)
- [ ] 아이콘/일러스트 라이선스 확인

## 2. 스토리 (시나리오, 대사)

- [ ] 세계관 (에테르넬리아) 정합
- [ ] 캐릭터 보이스 일관 (성격/말투)
- [ ] 한글 맞춤법 / 띄어쓰기
- [ ] 분기 컷씬의 트리거 조건 명시
- [ ] 등급 (12세) 준수

## 3. 퀘스트 (퀘스트 데이터, 진행 로직)

- [ ] 퀘스트 데이터 테이블 스키마 일치
- [ ] 진행 조건 / 보상 / 후속 퀘스트 트리거 검증
- [ ] 일일 퀘스트 리셋 시간 (UTC+9 자정)
- [ ] 보상이 밸런스 문서와 정합
- [ ] 로컬라이즈 키 누락 없음

## 4. 캐릭터 · 아이템 · 능력치 (코어 데이터)

- [ ] 능력치 공식 (`05-balance-philosophy.md`) 과 일치
- [ ] 직업별 스킬 데이터 일관
- [ ] 장비 옵션 한도 준수
- [ ] 강화 곡선 / 잠재 옵션 정합
- [ ] 데이터 테이블 정렬/주석 명확

## 5. 밸런스 (수치, 곡선)

- [ ] 시뮬레이션 결과 첨부
- [ ] 환생 도달 시간 분포 정상 범위 (3~20h)
- [ ] 인플레이션 위험 평가
- [ ] 변경 전후 비교 차트 (그래프 또는 표)
- [ ] 운영 영향 (기존 유저 너프/버프) 명시

## 6. 백엔드 / DB

- [ ] API 스펙 (`docs/api/`) 동기 갱신
- [ ] 마이그레이션 forward + (가능 시) rollback 제공
- [ ] 입력 검증 (Fastify schema)
- [ ] 인증 / 권한 체크
- [ ] 단위 테스트 (vitest) 커버리지 80%+
- [ ] 로그 / 메트릭 noise level 적절
- [ ] N+1 쿼리, 트랜잭션 경계 확인
- [ ] Docker 빌드 통과
- [ ] `.env.example` 갱신

## 7. QA (테스트, 회귀)

- [ ] 시나리오 테스트 케이스 (Given / When / Then)
- [ ] 회귀 테스트 — 기존 기능 영향 없음
- [ ] 엣지 케이스 (오프라인, 0/음수, 동시성)
- [ ] 재현 절차 명확 (스크린샷 / 로그)
- [ ] 자동화 가능 항목은 테스트 코드로

---

## 우선순위 태그 사용 가이드

- **[블로커]**: 머지 불가. 안전/정합성/규약 위반.
- **[중요]**: 가급적 본 PR 에서 수정. 운영 영향 큼.
- **[권장]**: 후속 PR 로 미뤄도 무방. 개선 제안.
- **[질문]**: 의도 파악용. 답변 후 다른 태그로 전환 가능.
- **[칭찬]**: 좋은 사례 강조. 학습 공유.

TM 종합 시 **블로커는 0개** 가 머지 조건.

---

## PR #32 Balance Checklist Addendum (Section 5)

- [ ] `tools/balance-sim/reports/balance-sim-report.md` includes 1000-run
  p10/median/p90 and min/max first-rebirth hours.
- [ ] Report confirms median is inside 5-10h and every sampled run is inside
  the 3-20h review band.
- [ ] Reward scaling table compares Stage 1-1 through 1-5 HP multiplier,
  reward multiplier, normal rewards, and 8x boss rewards.
- [ ] Simulator imports `reward.ts` and `stage.ts` rather than duplicating
  kill reward or stage multiplier constants.

## PR #33 Balance Checklist Addendum (Section 5)

- [ ] Balance report includes enhancement spend pressure for +0 through +50:
  minimum all-success cost, expected cost using `cost / successRate`, and
  expected hours against sampled median Lv50 gold/hour.
- [ ] Simulator imports `server/src/core/formulas/enhance.ts` rather than
  duplicating enhancement cost, success rate, or max-level constants.
- [ ] `docs/planning/05-balance-philosophy.md` documents MaxLevel 50, the
  `100 * (CurrentLevel + 1)^2` cost curve, 50-level success-rate curve, no
  downgrade or destruction on failure, and the `1 + EnhanceLevel * 0.1` stat payoff.
- [ ] Reward-vs-HP parity remains documented for Stage 1-1 through 1-5, with
  the 8x boss reward bonus reviewed separately from normal farming.

## PR #39 Balance Checklist Addendum (Section 4)

- [ ] `FEnhanceFormula::GetRarityCostMultiplier` and the server mirror keep
  None/Common/Uncommon/Rare/Epic/Legendary/Mythic multipliers at
  0/1/2/4/8/16/32.
- [ ] `TryEnhanceEquipped` spends the equipped item's rarity-scaled cost exactly
  once and preserves invalid-slot, empty-slot, max-level, and insufficient-gold
  guards.
- [ ] The HUD enhancement panel displays rarity-scaled cost for each equipped
  slot while Common remains compatible with the PR #33 cost curve.
- [ ] `tools/balance-sim` reports Common/Rare/Epic/Legendary/Mythic +0 to +50
  pressure and the eight-slot Legendary/Mythic expected cost against sampled
  Lv50 gold/hour.

## PR #44 Design Checklist Addendum (Section 1)

- [ ] Enhancement HUD rows display levels as `+N / 50`, with no hard-coded
  `+5` or `+MaxLevel` cap text.
- [ ] +50 rows use localized `MAX`/maximum copy for status, cost, and success
  fields instead of the empty-slot dash placeholder.
- [ ] Enhancement panel placement keeps the right-side 526px baseline,
  360-460px width clamp, and Canvas-height 1.0-2.0 scale for
  1080p/1440p/4K overlap checks.
- [ ] `ENHANCE_LEVEL_FORMAT`, `ENHANCE_SUCCESS_FORMAT`, and
  `ENHANCE_STATUS_MAX` remain present in both ko/en UI CSV files and pass
  `IdleProject.Localization.CsvIntegrity`.

## PR #44 Character Checklist Addendum (Section 4)

- [ ] `FEnhanceFormula::MaxEnhanceLevel` and `FItemInstance.EnhanceLevel`
  ClampMax are both 50.
- [ ] `GetEnhanceSuccessRate` uses the deterministic 50-level curve
  `clamp(0.95 - CurrentLevel * 0.018, 0.05, 0.95)`, with level 50 returning 0.
- [ ] `GetEnhanceCost(CurrentLevel, Rarity)` keeps
  `100 * (CurrentLevel + 1)^2 * RarityCostMultiplier` through level 49 and
  returns 0 at level 50+.
- [ ] `UInventoryComponent::ComputeEquipmentBonus` keeps
  `1 + EnhanceLevel * 0.1`, so Lv50 equipment applies a 6.0x multiplier.
- [ ] `TryEnhanceEquipped` and the HUD use `FEnhanceFormula::MaxEnhanceLevel`
  rather than hard-coded 5-level assumptions.

## PR #45 Character Checklist Addendum (Section 4)

- [ ] `EItemRarity` preserves existing numeric values and appends
  `Mythic = 6`.
- [ ] Drop rarity thresholds keep level 1 Mythic at 0% and make level 100
  Mythic available at 0.5%, with Common absorbing the residual probability.
- [ ] Mythic uses stat multiplier 4.5, enhancement cost multiplier 32, three
  affixes, and the same eligible item-set roll path as other non-common gear.
- [ ] HUD/localization maps Mythic to `RARITY_MYTHIC` and
  `Theme::RarityMythicStart`.
- [ ] UE Automation and server Vitest cover Mythic stat, rarity, affix, set,
  enhancement, HUD, localization, and Common-Legendary regression anchors.

## PR #46 Character Checklist Addendum (Section 4)

- [ ] `FRebirthFormula::GetRebirthPointsReward` returns 5 for count 0 at
  level 100, 13 for count 4 at level 100, 10 for count 0 at level 150, and
  18 for count 4 at level 150.
- [ ] `UIdleGameInstance::Rebirth` adds the formula reward before incrementing
  rebirth count and preserves the existing level, exp, stat allocation, gold
  10%, chapter boss gate, and stage reset behavior.
- [ ] `PreviewRebirthReward` uses the same formula inputs as the next
  `Rebirth` call so HUD preview and applied reward cannot diverge.
- [ ] The first level-100 rebirth remains a 5-point legacy-compatible reward.

## PR #42 Balance Checklist Addendum (Section 5)

- [ ] `tools/balance-sim/reports/balance-sim-report.md` includes `Pet Feed
  Gold Pressure` with 1000-run sampled Lv50 median gold/hour.
- [ ] Pet feed pressure imports `server/src/core/formulas/petLevel.ts` instead
  of duplicating `MAX_PET_LEVEL`, `getFeedCost`, or bonus multiplier constants.
- [ ] Report shows Lv0 to Lv10 total feed cost of 192,500 gold and the
  per-level cumulative feed-cost table.
- [ ] Report compares dog gold bonus 20% to 40%, bird drop bonus 15% to 30%,
  and dog payback of about 1.47h at 654,689 median Lv50 gold/hour.
- [ ] `docs/planning/05-balance-philosophy.md` documents the formula,
  compounding/payback interpretation, and gold-sink guardrails.

## PR #40 Character Checklist Addendum (Section 4)

- [ ] `FItemInstance` keeps affix defaults at zero for legacy item regression.
- [ ] `FDropFormula::RollAffixes` uses injected `FRandomStream` and enforces
  Common 0, Uncommon/Rare 1, Epic 2, Legendary 2-3, Mythic 3 affixes.
- [ ] `UInventoryComponent::ComputeEquipmentBonus` applies enhanced affix
  bonuses to CritRate, AtkSpeed, and MagicAtk.
- [ ] `FItemPowerScore::Compute` includes CritRate, AtkSpeed, and MagicAtk
  weights while zero-affix items keep the previous score.
- [ ] UE Automation covers affix rolls, equipment bonus propagation, derived
  stat propagation, PowerScore weighting, and zero-affix regression.

## PR #41 Character Checklist Addendum (Section 4)

- [ ] `AIdleCharacter::RefreshDerivedStats()` caches the same final primary and
  derived stats used to initialize combat.
- [ ] `GetCurrentPrimaryStats()`, `GetCurrentDerivedStats()`, and
  `GetCurrentLevel()` are `BlueprintPure` and do not recompute formulas.
- [ ] Cached derived stats include equipment, allocated stat points, rebirth
  bonus points, and skill passive effects.
- [ ] UE Automation covers getter values after refresh and combat stat
  initialization parity.

## PR #43 Character Checklist Addendum (Section 4)

- [ ] `FItemInstance` defaults `ItemSet` to `EItemSet::None`, and set membership
  does not change `FItemPowerScore::Compute`.
- [ ] `FDropFormula::RollItemSet` is deterministic with injected
  `FRandomStream`, keeps Common/None as no-set, and assigns Rare+ items to
  Warrior, Guardian, or Arcane.
- [ ] `FSetBonusFormula::ComputeSetBonus` applies tiered 2-piece and 4-piece
  flat `FDerivedStats` bonuses, with 4-piece including the 2-piece bonus.
- [ ] `UInventoryComponent::ComputeEquipmentBonus` adds set bonuses after
  per-item enhanced bonuses, with under-threshold and None-set equipment
  preserving legacy totals.
- [ ] UE Automation covers set roll behavior, 2-piece/4-piece thresholds,
  equipment bonus propagation, derived stat propagation, under-threshold
  regression, and PowerScore regression.

## PR #43 Designer Checklist Addendum (Section 1)

- [ ] Equipment set HUD shows active set name, 2/4-piece tier, active bonus,
  and next threshold without overlap at 1080p, 1440p, and 4K.
- [ ] Equipment set HUD uses UI localization keys instead of hard-coded
  display text.

## PR #49 Character Checklist Addendum (Section 4)

- [ ] `FCombatPowerFormula::ComputeCombatPower` maps final `FDerivedStats` to
  an `int64` CP value using weighted attack, defense, HP, crit, and attack
  speed terms, rounds the result, and clamps negative totals to zero.
- [ ] `AIdleCharacter::GetCombatPower()` is `BlueprintPure` and delegates to
  `ComputeCombatPower(GetCurrentDerivedStats())`.
- [ ] UE Automation covers formula anchors, zero/negative clamp behavior,
  character accessor parity, and monotonic CP increases from stat allocation,
  equipment affix/set bonuses, enhancement, rebirth bonus points, and
  transcend multiplier.

## PR #57 Character Checklist Addendum (Section 4)

- [ ] `EClassId` preserves existing numeric values for None and the first five
  classes, then appends Paladin=6, Berserker=7, and Summoner=8.
- [ ] `FClassGrowth` adds differentiated Paladin, Berserker, and Summoner
  profiles without changing Warrior, Mage, Archer, Thief, or Cleric anchors.
- [ ] `USkillComponent::LoadSkillsForClass` dispatches all eight classes and
  every default class skill set contains exactly 4 active, 2 passive, and 1
  ultimate skill.
- [ ] Skill DefinitionParity covers 8 classes x 7 skills, including
  classId/type/effect/cooldown/coefficient/buff/gauge/status/element fields.
- [ ] Summoner uses magic attack/magic defense in combat formula and skill
  damage routing, while Paladin and Berserker remain physical.

## PR #60 Balance Checklist Addendum (Section 5)

- [ ] `tools/balance-sim` class DPS uses `computeClassDamage` for base hits and
  active skill damage under the shared Lv-scaled review defense.
- [ ] Lv50 and Lv100 DPS classes stay within +/-15% of the DPS median.
- [ ] Paladin and Cleric stay below 0.8x DPS median while preserving tank/healer
  HP, defense, or MagicDef compensation.
- [ ] `client/Content/Data/SkillDB.csv`, `USkillComponent`, and server
  `skills.ts` keep Berserker and Summoner tuned coefficients in parity.
- [ ] The balance report includes the 1000-run first-rebirth distribution and
  the Lv50/Lv100 class balance table.

## PR #50 Character Checklist Addendum (Section 4)

- [ ] `FTowerFormula::GetFloorRequiredPower` keeps floor 1 at 100 CP and uses
  `round(100 * 1.15^(max(1, Floor) - 1))`.
- [ ] `FTowerFormula::CanClearFloor` is an exact `CombatPower >= RequiredPower`
  gate, including one-below and exact-boundary tests.
- [ ] `FTowerFormula::GetFloorReward` keeps the V1 linear
  `round(50 * max(1, Floor))` gold reward.
- [ ] `UTowerService::TryClimbTower` advances only newly clearable consecutive
  floors, sums rewards once, broadcasts `OnTowerClimbed` only when progress is
  made, and caps one call at 100 floors.
- [ ] `UIdleGameInstance::ClimbTower` reads current
  `AIdleCharacter::GetCombatPower()`, applies successful tower rewards through
  `AddGold`, and safely returns zero when no player character is available.

## PR #43 QA Checklist Addendum (Section 7)

- [ ] QA scenario covers 2-piece active, 4-piece complete, under-threshold,
  None-set regression, and ko/en CSV integrity in Given / When / Then format.

## PR #65 Character Checklist Addendum (Section 4)

- [ ] `EItemRarity` is exactly None=0, Common=1, Rare=2, Epic=3, Unique=4,
  Legendary=5, Transcendent=6, Mythic=7, with no `Uncommon` references in
  `client/Source` or `server/src`.
- [ ] `FRarityMigration::MigrateLegacy` maps legacy values 1/2/3/4/5/6 to
  Common/Rare/Rare/Epic/Legendary/Mythic and `ApplyFromSave` runs this only
  for `SaveVersion < 7`.
- [ ] Save capture writes `SaveVersion = 7`, cloud `maxEquipmentGrade` accepts
  0-7, and v7 saves are not migrated a second time.
- [ ] Drop, enhance, rune, class rune, item set, rune set, and rune codex
  formulas all cover the seven rarity rows with client/server parity.
- [ ] Rune codex dimensions are 63 total cells, 35 core cells, and 28 util
  cells; migrated legacy value 2 and 3 codex entries merge into the new Rare
  row while Unique and Transcendent start locked.

## PR #65 Designer Checklist Addendum (Section 1)

- [ ] `docs/planning/04-art-direction.md` and
  `docs/planning/ui-tokens.json` list exactly seven item rarity tokens:
  Common, Rare, Epic, Unique, Legendary, Transcendent, and Mythic.
- [ ] `Uncommon` is absent from active rarity tokens and UI localization keys.
- [ ] ko/en UI labels remain paired as 일반/Common, 희귀/Rare, 영웅/Epic,
  유니크/Unique, 전설/Legendary, 초월/Transcendent, and 신화/Mythic.
- [ ] HUD rarity rendering uses token-driven color only; Unique keeps the
  legacy green, Transcendent uses cyan, and Mythic keeps the orange-to-blue
  gradient fallback.
- [ ] Item rarity `초월` appears only in item/rune context and does not reuse or
  replace prestige `TRANSCEND_*` copy.
