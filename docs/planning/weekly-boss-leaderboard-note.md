# 주간 보스 데미지 리더보드 — 밸런스 노트 (PR #79)

> PM/Claude 직접 작성(Codex 사용량 한도 소진 + balance 서브에이전트 API 529 과부하 대체). 리더보드는 **읽기 전용 표시**이며 점수 공식 변경 없음.

## 1. 랭킹 모델
- **점수 = 주간 보스 누적 데미지**(#77) = `getChallengeDamage(CP) × 도전 횟수`(주 최대 7회). 즉 한 주에 쌓은 보스 데미지 총합.
- **랭킹 키 = ISO week 문자열**(`UQuestService::GetCurrentUtcWeekString`, 예 `2026-W22`) — 주 단위 자동 분리/리셋. 매주 새 보드.
- **기록 시점**: 세이브 업로드(#54) 시 서버 `save.service`가 payload의 `weeklyBossWeekId`/`weeklyBossDamage`를 `leaderboard_weekly_damage`에 upsert. 별도 제출 경로 없음(기존 updatePower/updateRebirth와 동일 패턴).
- **저장**: PG `leaderboard_weekly_damage(week_id, character_id, damage)`. bigint 정밀도 위해 Redis ZSET 미사용·PG 직접 조회(power/rebirth 캐시 경로 불변).

## 2. 밸런스 영향 — 없음
- 리더보드는 **순위 조회/표시 전용**. 데미지/보상/성장 공식 일절 변경 없음.
- 점수는 기존 #77 주간 보스 데미지(= CP 파생)를 그대로 사용 → 코어 페이싱·경제 비침범.
- **balance-sim 재실행 (무변동 확인)**: median **5.328h** (p10 4.919 / p90 5.751, min/max 4.564 / 6.144, status `inside-target`). 첫 환생 페이싱 변화 없음.

## 3. 후속
- **주간 랭크 보상**: 현재 주간 보드는 보상이 없음(조회만). 주간 종료 시 순위 보상은 시즌 경쟁 보상(#78 예정) 패턴을 주간 단위로 적용하는 후속 슬라이스로 분리.
- 역대 주간 명예의 전당, 실시간 갱신.
