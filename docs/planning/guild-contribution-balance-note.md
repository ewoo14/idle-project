# 길드 기여/레벨/버프 밸런스 노트 — PR-G2

> 대상: PR-G2 (기여도/길드 레벨·버프/상점) · 작성 2026-05-30 · 선행 PR-G1(#85, SaveVer17)
> 범위: 기여 3종(출석/헌납/전투·던전 자동) + 길드 레벨/영구 버프 + 길드 상점. **길드 보스 데미지 기여=PR-G3.**

## 1. 기여도 발생원 (3종, EXP·개인 포인트 동시 적립)

| 발생원 | 수치 | 상한 | 비고 |
| --- | --- | --- | --- |
| 일일 출석 | +50 | 1일 1회(UTC) | 낙오 멤버 최소 기여 보장 |
| 재화 헌납 | `floor(gold/1000)` | 일일 500 기여 | **골드 클라 권위** → 서버 미차감, 일일 상한으로 어뷰즈 억제 |
| 전투/던전 자동 | 던전 클리어 +5 / 보스 도전 +10 | 주간 2000 기여 | "그냥 플레이하면 쌓임"(비동기 협력 핵심) |

- 모든 기여는 `guild.exp += amount`(길드 성장) + `member.contribution_points += amount`(상점 화폐) + `member.weekly_contribution += amount`(주간 랭킹·리셋, G3) 동시 적립.
- 주간 리셋: 새 ISO week 진입 시 weekly·주간 자동 상한 리셋(멤버별 `weekly_reset_id` lazy, quest 패턴). **exp·contribution_points는 영속**.

## 2. 길드 레벨 (무한 기하)

```
GuildLevelThreshold(L) = floor(GUILD_LEVEL_BASE * GUILD_LEVEL_GROWTH^(L-1))  # 레벨 L→L+1 구간 비용
누적 임계: getGuildLevel(exp) = exp가 Σ_{i=1..L} threshold(i) 를 넘는 최대 L+1
GUILD_LEVEL_BASE = 10000 ; GUILD_LEVEL_GROWTH = 1.6
```

경계 예: L1(0~9999) / L2(10000~) / L3(26000~) / L4(51600~). 무한 성장([[project_infinite_growth]]).

## 3. 길드 영구 버프 (전 멤버, 로컬 캐시 적용)

```
GuildBuff(L): 공격력 +GUILD_BUFF_PER_LEVEL*L , 골드획득 +GUILD_BUFF_PER_LEVEL*L
GUILD_BUFF_PER_LEVEL = 0.004  (레벨당 +0.4%)
```

- **단일 적용 지점**(이중 적용 가드, [[feedback_substantial_slices]]/#72 교훈): 공격력=`RefreshDerivedStats` 배수, 골드=`AddGold` 배수. 기존 마스터리/펫/소비 버프와 합산.
- 오프라인: 마지막 동기화 버프를 세이브 캐시로 적용.
- **경제 영향**: 레벨당 +0.4%는 완만 — 길드 레벨 25면 +10%. 무한 곡선이나 길드 EXP 누적 난도(1.6^)로 페이싱. 솔로 플레이 대비 길드 가입 유인이되 파워크리프 과도하지 않게 초기값 보수적. median 페이싱 영향은 G2 통합 검증에서 재측정.

## 4. 길드 상점 (개인 기여 포인트 소비, 6종 소비형)

| 아이템 | 효과 | 가격(포인트) |
| --- | --- | --- |
| 강화석 / 강화석 묶음 | 강화재(#71) | 소/대 |
| 골드 주머니 | 즉시 골드 | 중 |
| 경험치 물약 | 즉시 EXP | 중 |
| 펫 먹이 | 펫 성장(#69) | 소 |
| 정수 | 룬 에센스(#61) | 중 |

- 소비형(영구 해금 아님) → 테이블 없이 포인트 차감 + 보상 응답, 클라가 캐릭터 세이브 반영. 정확한 가격/수량은 구현 카탈로그 상수 기준, 경제 밸런스 재튜닝 여지.

## 5. SaveVersion

- **17 → 18**: 클라 캐시 `CachedGuildLevel`/`CachedGuildAttackPct`/`CachedGuildGoldPct`/`CachedContributionPoints`/`PendingAutoContribution`/`LastGuildAttendanceDate`. 오프라인 버프 적용·기여 델타 보존.

## 6. 비고 / 후속

- 자동 기여 단위(+5/+10)·주간 상한(2000)은 초기값 — 길드 활동량 데이터로 재튜닝.
- 길드 보스 데미지 기여(4번째 발생원)·주간 길드 랭킹 = PR-G3.
