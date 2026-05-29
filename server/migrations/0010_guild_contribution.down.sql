alter table guild_members
  drop column if exists weekly_auto_contribution,
  drop column if exists last_donation_date,
  drop column if exists daily_donation,
  drop column if exists weekly_reset_id;
