create table if not exists leaderboard_weekly_damage (
  week_id text not null,
  character_id uuid not null references characters(id) on delete cascade,
  damage bigint not null default 0,
  updated_at timestamptz not null default now(),
  primary key (week_id, character_id)
);

create index if not exists leaderboard_weekly_damage_week_damage
  on leaderboard_weekly_damage (week_id, damage desc);
