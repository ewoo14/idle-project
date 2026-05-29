-- 길드 기초 PR-G1: 길드 본체 / 멤버십 / 승인 신청
-- 1캐릭터 1길드(guild_members.character_id UNIQUE), member_count denorm 정합.

create table if not exists guilds (
  id uuid primary key,
  name varchar(16) not null unique,
  notice text not null default '',
  join_mode text not null default 'open' check (join_mode in ('open', 'approval')),
  level int not null default 1 check (level >= 1),
  exp bigint not null default 0 check (exp >= 0),
  master_character_id uuid not null references characters(id) on delete cascade,
  member_count int not null default 1 check (member_count >= 0),
  week_id text,
  created_at timestamptz not null default now()
);

create table if not exists guild_members (
  guild_id uuid not null references guilds(id) on delete cascade,
  character_id uuid primary key references characters(id) on delete cascade,
  rank text not null default 'member' check (rank in ('master', 'vice', 'officer', 'member')),
  joined_at timestamptz not null default now(),
  weekly_contribution bigint not null default 0 check (weekly_contribution >= 0),
  total_contribution bigint not null default 0 check (total_contribution >= 0),
  contribution_points bigint not null default 0 check (contribution_points >= 0),
  last_attendance_date text
);

create index if not exists guild_members_guild_idx
  on guild_members (guild_id);

create table if not exists guild_join_requests (
  guild_id uuid not null references guilds(id) on delete cascade,
  character_id uuid not null references characters(id) on delete cascade,
  requested_at timestamptz not null default now(),
  primary key (guild_id, character_id)
);

create index if not exists guild_join_requests_character_idx
  on guild_join_requests (character_id);
