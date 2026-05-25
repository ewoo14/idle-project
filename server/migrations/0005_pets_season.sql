create table if not exists pet_state (
  user_id uuid primary key references users(id) on delete cascade,
  owned_pet_ids jsonb not null default '["dog", "cat"]'::jsonb,
  equipped_pet_id varchar(64),
  updated_at timestamptz not null default now(),
  constraint pet_state_owned_pet_ids_array check (jsonb_typeof(owned_pet_ids) = 'array')
);

create table if not exists season_state (
  user_id uuid not null references users(id) on delete cascade,
  season_id int not null default 1,
  tokens int not null default 0 check (tokens >= 0),
  claimed_tiers jsonb not null default '[]'::jsonb,
  updated_at timestamptz not null default now(),
  primary key (user_id, season_id),
  constraint season_state_claimed_tiers_array check (jsonb_typeof(claimed_tiers) = 'array')
);

create index if not exists season_state_tokens_idx
  on season_state (season_id, tokens);
