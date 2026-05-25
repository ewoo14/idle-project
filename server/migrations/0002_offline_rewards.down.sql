alter table characters
  drop column if exists last_seen_at,
  drop column if exists total_exp,
  drop column if exists gold;
