# 길드 보스/주간 랭킹 밸런스 노트 — PR-G3

> 대상: PR-G3 (길드 보스 공유 HP 풀 + 주간 길드 랭킹) · 작성 2026-05-30 · 선행 G1(#85)/G2(#86, SaveVer18)
> **길드 시스템(백로그 #80) 완결 슬라이스.** 재사용: #77 주간 보스(공유 HP/마일스톤), #79 주간 랭킹 쿼리.

## 1. 길드 보스 (공유 HP 풀, 주간)

```
GUILD_BOSS_BASE_HP   = 1,000,000
GUILD_BOSS_HP_GROWTH = 1.5
GetGuildBossHp(defeatedCount) = floor(BASE * GROWTH^max(defeatedCount,0))
  → 1격 1,000,000 / 2격 1,500,000 / 3격 2,250,000 ... (무한 기하)
GetChallengeDamage(cp) = max(0, cp)                  # #77 재사용
WEEKLY_GUILD_BOSS_CHALLENGE_LIMIT = 7 (멤버당 주간 도전)
```

- **비동기 협력**: 멤버 각자 도전 → `accum_damage` 공유 누적. `accum >= HP`면 격파(`defeated_count++`), 잔여 데미지 **이월**(다음 보스 HP로). 누적이 여러 임계를 한 번에 넘으면 연속 격파.
- 주간 리셋(ISO week): 새 week → 보스 누적/격파/멤버 도전 카운트 리셋.

## 2. 보스 데미지 → 기여 (4번째 발생원)

```
기여 = floor(damage / GUILD_BOSS_DMG_TO_CONTRIB)   # DMG_TO_CONTRIB = 10,000
→ 서버 applyContribution(G2) 단일 적립(exp/points/weekly), 클라 이중적립 없음
```

- G2의 출석/헌납/전투·던전 자동에 이어 **보스 데미지가 4번째 기여원** → 길드 레벨·상점·주간 랭킹에 모두 반영.

## 3. 격파 보상 (전원, 격파 N건 누적)

```
GUILD_BOSS_REWARD_PER_DEFEAT = [ gold 200,000 , essence 5 ]   # 격파 1건당
claim: 현 주 (defeated_count - 멤버 수령분) × 보상 누적 지급
```

- **클라 적용 가능한 실존 재화만**(gold→AddGold, essence→RuneEssence) — G2 `ApplyGuildShopReward` 재사용. (1차 설계의 `contributionPoints`는 서버 컬럼이라 클라 지급 불가 → essence로 교정.)
- 멤버별 `boss_claimed_count`+주간 추적, 동시성 가드(조건부 update 멱등).

## 4. 주간 길드 랭킹

```
길드 점수 = Σ member.weekly_contribution (길드별)
GET /rankings: 상위 N 정렬(동점 created_at asc 안정) + 내 길드 순위(rank() over)
```

- #79 `row_number()/rank() over` 패턴 재사용. 클라는 #76 리더보드 UI 재사용(상위 길드·내 순위).
- 주간 랭킹 보상은 후속(시즌 보상 #78 패턴) — 본 PR은 랭킹 표시까지.

## 5. SaveVersion

- **18 → 19**: 보스 진행 표시 캐시(`CachedBossDefeatedCount`/`CachedBossChallengesRemaining`). 보스 상태 자체는 서버 권위(스냅샷), 클라는 표시용 캐시.

## 6. 경제/페이싱

- 보스 HP 1.5^ 곡선 + 주당 7도전 한도로 길드 규모·CP에 비례한 격파 페이싱. 보상(gold 200k+essence 5/격파)은 길드 협력 인센티브이되 솔로 파밍(던전/주간보스) 대비 과도하지 않게 초기값. 통합 median 페이싱 영향 재측정 대상.
- 데미지→기여 환산(/10000)·도전 한도·보상 수치는 초기값, 길드 활동 데이터로 재튜닝.
