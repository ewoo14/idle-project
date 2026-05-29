-- 길드 기초 PR-G3: 공유 HP 풀 길드 보스 + 보스 데미지 기여(4번째 발생원) + 주간 길드 랭킹.
-- 주간 길드 랭킹은 guild_members.weekly_contribution 합 집계로 산출(별도 테이블 없음, #76 패턴).

-- 공유 보스(길드당 1행). 주간(week_id) 단위로 누적/격파를 추적, 새 주 진입 시 lazy 리셋.
create table if not exists guild_boss (
  guild_id uuid primary key references guilds(id) on delete cascade,
  week_id text not null,
  accum_damage bigint not null default 0 check (accum_damage >= 0),
  defeated_count int not null default 0 check (defeated_count >= 0),
  updated_at timestamptz not null default now()
);

-- 보스 데미지 기여(주간 내부 랭킹용). 멤버별 누적 데미지.
create table if not exists guild_boss_contrib (
  guild_id uuid not null references guilds(id) on delete cascade,
  week_id text not null,
  character_id uuid not null references characters(id) on delete cascade,
  damage bigint not null default 0 check (damage >= 0),
  primary key (guild_id, week_id, character_id)
);

create index if not exists guild_boss_contrib_guild_week
  on guild_boss_contrib (guild_id, week_id, damage desc);

-- 멤버별 주간 보스 도전 횟수(주간 리셋)와 격파 보상 수령 마일스톤 추적.
-- weekly_reset_id(G2) 기준으로 weekly_boss_challenges 를 lazy 리셋,
-- boss_claim_week_id 가 현재 주와 다르면 boss_claimed_count 를 0 부터 재집계.
alter table guild_members
  add column if not exists weekly_boss_challenges int not null default 0
    check (weekly_boss_challenges >= 0),
  add column if not exists boss_claimed_count int not null default 0
    check (boss_claimed_count >= 0),
  add column if not exists boss_claim_week_id text;
