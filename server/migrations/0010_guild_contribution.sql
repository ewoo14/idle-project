-- 길드 기초 PR-G2: 기여도(출석/헌납/자동) 일일·주간 상한 추적 컬럼.
-- 길드 EXP·레벨·개인 기여 포인트는 G1 컬럼(guilds.exp/level, guild_members.contribution_points)을 재사용.
-- 상점 구매는 소비형(포인트 차감 + 응답 지급)이라 별도 테이블 없음.

alter table guild_members
  add column if not exists weekly_auto_contribution bigint not null default 0
    check (weekly_auto_contribution >= 0),
  add column if not exists last_donation_date text,
  add column if not exists daily_donation bigint not null default 0
    check (daily_donation >= 0),
  -- 멤버별 주간 리셋 마커(ISO week). quest.weekly_reset_id 패턴 — 멤버 활동 시 lazy 리셋.
  add column if not exists weekly_reset_id text;
