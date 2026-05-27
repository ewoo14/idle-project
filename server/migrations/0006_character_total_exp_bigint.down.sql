alter table characters
  alter column total_exp type integer using least(total_exp, 2147483647)::integer;
