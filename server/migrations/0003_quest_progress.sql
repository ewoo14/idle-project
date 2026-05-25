create table if not exists quest_progress (
  user_id uuid not null references users(id) on delete cascade,
  quest_id varchar(64) not null,
  progress int not null default 0 check (progress >= 0),
  completed boolean not null default false,
  claimed boolean not null default false,
  daily_reset_date date,
  updated_at timestamptz not null default now(),
  primary key (user_id, quest_id)
);

create index if not exists quest_progress_user_completed_idx
  on quest_progress (user_id, completed, claimed);

create index if not exists quest_progress_daily_reset_idx
  on quest_progress (daily_reset_date);
