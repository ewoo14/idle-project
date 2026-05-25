create extension if not exists citext;

create table if not exists users (
  id uuid primary key,
  email citext not null unique,
  password_hash text not null,
  nickname varchar(16) not null unique,
  created_at timestamptz not null default now(),
  last_login_at timestamptz
);

create table if not exists characters (
  id uuid primary key,
  user_id uuid not null references users(id) on delete cascade,
  class_id int not null check (class_id between 1 and 5),
  level int not null default 1 check (level between 1 and 200),
  rebirth_count int not null default 0,
  stats jsonb not null default '{}'::jsonb,
  skill_tree jsonb not null default '{}'::jsonb,
  inventory jsonb not null default '[]'::jsonb,
  last_save_at timestamptz
);

create table if not exists saves (
  id uuid primary key,
  character_id uuid not null references characters(id) on delete cascade,
  version int not null,
  payload jsonb not null,
  server_validated boolean not null default false,
  created_at timestamptz not null default now()
);

create table if not exists leaderboard_power (
  character_id uuid primary key references characters(id) on delete cascade,
  season_id int not null,
  power_score bigint not null default 0,
  updated_at timestamptz not null default now()
);

create table if not exists leaderboard_rebirth (
  character_id uuid primary key references characters(id) on delete cascade,
  season_id int not null,
  rebirth_count int not null default 0,
  updated_at timestamptz not null default now()
);

create index if not exists leaderboard_power_season_score on leaderboard_power (season_id, power_score desc);
create index if not exists leaderboard_rebirth_season_count on leaderboard_rebirth (season_id, rebirth_count desc);
create index if not exists saves_character_created on saves (character_id, created_at desc);
