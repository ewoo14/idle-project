drop index if exists quest_progress_weekly_reset_idx;

alter table quest_progress
  drop column if exists weekly_reset_id;
