# 길드 기초 PR-G2 구현 계획 — 기여도/길드 레벨·버프/상점

> 스펙: [`2026-05-29-guild-foundation-design.md`](../specs/2026-05-29-guild-foundation-design.md) §3-a(G2행)/§4/§5. 선행 PR-G1(#85) 머지 완료(SaveVer 17). v3 디스패치, 현행 코드 재검증.

**Goal:** 길드 2차 슬라이스 — 기여도 3종(일일 출석·재화 헌납·전투/던전 자동) 집계 → 길드 EXP·길드 레벨·**전 멤버 영구 버프(로컬 적용)** + 개인 기여 포인트·길드 상점. 주간 리셋. SaveVer 17→18. (길드 보스 데미지 기여=G3)

**Architecture:** 서버 `guild` 모듈 확장(기여 라우트·길드 레벨·상점) + 주간 리셋(ISO week 재사용). 클라 GuildService 버프 캐시·오프라인 적용 + 기여 델타 누적/플러시 + 상점/헌납/출석 UI.

**Tech Stack:** G1과 동일.

## 기여/레벨/버프 공식 (스펙 §4, 초기값 — balance-note 확정)
```
출석:   +50 / UTC일 1회
헌납:   floor(gold/1000), 일일 상한 500 기여
자동:   던전 클리어·보스 도전당 +k(소량), 주간 상한
→ guild.exp += 기여 ; member.contribution_points += 기여 ; member.weekly_contribution += 기여
GuildLevelThreshold(L) = floor(10000 * 1.6^(L-1))   # 누적 exp→레벨
GuildBuff(L): 공격력 +0.4%*L, 골드획득 +0.4%*L  (로컬 캐시, 오프라인 적용)
주간 리셋: 새 week_id → weekly_contribution/주간 자동 상한 리셋(exp·points는 영속)
```

## Task 1: backend (backend)
- [ ] 마이그레이션(0010): `guild_members`에 `weekly_auto_contribution bigint default 0`(자동 기여 주간 상한 추적), `last_donation_date text`/`daily_donation bigint default 0`(헌납 일일 상한) 추가. 길드 상점 구매는 포인트 차감만(소비형 보상은 캐릭터 세이브 반영)이라 별도 테이블 불필요 — 단 영구 해금형 상점이면 `guild_shop_purchases(character_id, item_id, count)` 추가(설계: **소비형**으로 단순화 → 테이블 없이 포인트 차감 + 응답으로 지급).
- [ ] `guild.service.ts` 확장: `attendance(characterId)`(last_attendance_date 비교 1일 1회, +50 적립), `donate(characterId, gold)`(일일 상한 검증, floor(gold/1000) 적립), `contribute(characterId, autoAmount)`(주간 자동 상한 검증, 적립), 공통 `applyContribution(member, amount)`(guild.exp += , points += , weekly += , 주간/일자 리셋 선처리). `shopBuy(characterId, itemId)`(카탈로그 가격 검증·포인트 차감·보상 반환).
- [ ] `GuildLevelFormula`(순수): `getGuildLevel(exp)`, `getGuildBuff(level)` → {attackPct, goldPct}. **클라 parity 상수 분리**(BASE=10000, GROWTH=1.6, BUFF_PER_LEVEL=0.004).
- [ ] 길드 상점 카탈로그(서버 상수): 강화재/소비아이템 N종 + 포인트 가격.
- [ ] 라우트: `POST /:id/attendance`, `POST /:id/donate`, `POST /:id/contribute`, `GET /:id/shop`, `POST /:id/shop/:itemId/buy`. snapshot(`GET /me`)에 guild level/buff·내 points·출석/헌납 가능여부 추가.
- [ ] vitest: 출석 1일1회·중복거부, 헌납 일일상한, 자동 주간상한, exp→레벨 임계, 주간 리셋(weekly 0·exp 유지), 상점 구매·포인트 부족 거부. `cd server; npm run lint && test -- guild && build` GREEN.
- [ ] 커밋 `feat: 길드 기여/레벨/상점 backend (PR-G2)`.

## Task 2: client (character)
- [ ] `GuildFormula` 확장: `GetGuildLevel(int64 Exp)`, `GetGuildBuff(int32 Level)`(서버 parity 상수 1:1) → FGuildBuff{AttackPct, GoldPct}.
- [ ] `GuildTypes`: FGuildSnapshot에 GuildLevel/GuildExp/ContributionPoints/버프·출석가능·헌납가능 필드 추가. FGuildBuff 구조체.
- [ ] `GuildService`: 버프 캐시 `CachedBuff`, 접근자 `GetGuildBuff()`. 기여 델타 누적 `AddPendingAutoContribution(int64)`/`ConsumePendingAutoContribution()`. ApplySnapshot 시 버프/레벨/포인트 갱신.
- [ ] **버프 적용 지점**: `IdleGameInstance`(또는 스탯/골드 계산) — RefreshDerivedStats 공격력 배수에 `*(1+AttackPct)`, AddGold 골드획득에 `*(1+GoldPct)` 합산. **기존 마스터리/펫 등 전역 버프와 합산 지점 일관**(이중 적용 금지, #72 교훈). 오프라인엔 캐시 버프 적용.
- [ ] ApiClient: Attendance/Donate/Contribute/GetShop/BuyShopItem. IdleGameInstance: 던전 클리어·보스 도전 시 `AddPendingAutoContribution`, 세이브/재접속 시 contribute 플러시 → RefreshGuildSnapshot.
- [ ] SaveVer **17→18**: `CachedGuildLevel`(int32), `CachedGuildAttackPct`/`CachedGuildGoldPct`(float 또는 int 천분율), `CachedContributionPoints`(int64), `PendingAutoContribution`(int64), `LastGuildAttendanceDate`(FString). 전 세이브 테스트 단언 18로 일괄 갱신.
- [ ] Automation: 버프 parity(레벨→%), CP/골드 버프 반영, 델타 누적·플러시, 오프라인 버프 적용, 세이브 v18 라운드트립. 익명 헬퍼 Guild~ prefix grep.
- [ ] 커밋 `feat: 길드 기여/버프 클라 + SaveVer18 (PR-G2)`.

## Task 3: UI (designer)
- [ ] 내 길드 패널 확장: 길드 레벨/EXP 진행, 현재 버프(공격/골드 %), 내 기여 포인트. **일일 출석 버튼**, **헌납 버튼**(보유 골드 일부 헌납 — 프리셋 금액). **길드 상점**(아이템 목록·가격·구매). 주간 기여 표시.
- [ ] ko/en 로컬라이즈 + CsvIntegrity. 표준 jumbo 빌드.
- [ ] 커밋 `feat: 길드 기여/상점/버프 UI (PR-G2)`.

## Task 4: balance/docs (balance)
- [ ] `docs/planning/guild-contribution-balance-note.md`: 기여 3종 수치(출석 50/헌납 floor(gold/1000) 일상한 500/자동 k·주상한), 길드 레벨 곡선(10000×1.6^), 버프 곡선(+0.4%/Lv 공격·골드), 상점 카탈로그·가격, 주간 리셋. CP/경제 영향 분석(버프가 무한성장 곡선에 주는 영향).
- [ ] [[project_pr_order]]/[[project_session_progress]] 갱신(PR-G2/SaveVer18). 커밋 `docs: 길드 기여 G2 밸런스 노트 (PR-G2)`.

## Task 5: qa (qa)
- [ ] E2E: 출석(1일1회)→헌납(상한)→자동 기여(던전/보스 도전 후 플러시)→길드 EXP·레벨업→전 멤버 버프 CP 반영→상점 구매(포인트 차감/부족 거부)→주간 리셋. cross-DB 정합. 세이브 v18. 표준 jumbo + ue-automation.ps1 게이트. PR 스크린샷.
- [ ] 커밋 `test: 길드 기여 G2 E2E (PR-G2)`.

## Self-Review
- 스펙 §3-a G2행 라우트(attendance/donate/contribute/shop) 전부 Task1 ✓. 버프 로컬 적용 Task2 ✓. UI Task3 ✓.
- parity: 서버 `getGuildLevel`/`getGuildBuff` ↔ 클라 `GetGuildLevel`/`GetGuildBuff`(BASE 10000/GROWTH 1.6/BUFF 0.004) — TM cross-check.
- **이중 적용 가드(#72 교훈)**: 길드 버프는 RefreshDerivedStats 공격 배수 + AddGold 골드 단일 지점만, getter 중복 금지.
- 보스 데미지 기여=G3(본 PR 미포함). SaveVer 17→18 전 테스트 단언 갱신(stale 방지).
- jumbo ODR 주의([[reference_ue_headless_verify]] §1-b).

## 매핑: 1→backend, 2→character(메인), 3→designer, 4→balance, 5→qa.
