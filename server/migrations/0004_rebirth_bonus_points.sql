alter table characters
  add column if not exists rebirth_bonus_points int not null default 0 check (rebirth_bonus_points >= 0);
