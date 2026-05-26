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

- [ ] Balance report includes enhancement spend pressure for +0 through +5:
  minimum all-success cost, expected cost using `cost / successRate`, and
  expected hours against sampled median Lv50 gold/hour.
- [ ] Simulator imports `server/src/core/formulas/enhance.ts` rather than
  duplicating enhancement cost, success rate, or max-level constants.
- [ ] `docs/planning/05-balance-philosophy.md` documents MaxLevel 5, the
  `100 * (CurrentLevel + 1)^2` cost curve, success rates, no downgrade or
  destruction on failure, and the `1 + EnhanceLevel * 0.1` stat payoff.
- [ ] Reward-vs-HP parity remains documented for Stage 1-1 through 1-5, with
  the 8x boss reward bonus reviewed separately from normal farming.

## PR #39 Balance Checklist Addendum (Section 4)

- [ ] `FEnhanceFormula::GetRarityCostMultiplier` and the server mirror keep
  None/Common/Uncommon/Rare/Epic/Legendary multipliers at 0/1/2/4/8/16.
- [ ] `TryEnhanceEquipped` spends the equipped item's rarity-scaled cost exactly
  once and preserves invalid-slot, empty-slot, max-level, and insufficient-gold
  guards.
- [ ] The HUD enhancement panel displays rarity-scaled cost for each equipped
  slot while Common remains compatible with the PR #33 cost curve.
- [ ] `tools/balance-sim` reports Common/Rare/Epic/Legendary +0 to +5 pressure
  and the eight-slot Legendary expected cost against sampled Lv50 gold/hour.

## PR #40 Character Checklist Addendum (Section 4)

- [ ] `FItemInstance` keeps affix defaults at zero for legacy item regression.
- [ ] `FDropFormula::RollAffixes` uses injected `FRandomStream` and enforces
  Common 0, Uncommon/Rare 1, Epic 2, Legendary 2-3 affixes.
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
