alter table quest_progress
  add column if not exists weekly_reset_id varchar(8);

create index if not exists quest_progress_weekly_reset_idx
  on quest_progress (weekly_reset_id);
