alter table guild_members
  drop column if exists weekly_boss_challenges,
  drop column if exists boss_claimed_count,
  drop column if exists boss_claim_week_id;

drop table if exists guild_boss_contrib;
drop table if exists guild_boss;
