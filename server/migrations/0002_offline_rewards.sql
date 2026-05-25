alter table characters
  add column if not exists gold int not null default 0 check (gold >= 0),
  add column if not exists total_exp int not null default 0 check (total_exp >= 0),
  add column if not exists last_seen_at timestamptz;
